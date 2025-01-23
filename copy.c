#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include "commands.h"
#include "utils.h"
#include "threads.h"

typedef struct
{
    bool flag;
    int exit_code;
    pthread_mutex_t *mutex;
    Arena *arena;
} WorkerArgs;

typedef struct
{
    char *source;
    char *destination;
} Job;

int copy_file(FILE *f_source, FILE *f_dest)
{
    char buffer[BUFSIZ];
    size_t bytes_read;

    if (errno == EACCES)
    {
        fprintf(stderr, "Permission denied\n");
        fclose(f_source);
        fclose(f_dest);
        return EXIT_FAILURE;
    }
    if (f_source == NULL || f_dest == NULL)
    {
        fprintf(stderr, "Error opening file(s)\n");
        fclose(f_source);
        fclose(f_dest);
        return EXIT_FAILURE;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f_source)) > 0)
    {
        fwrite(buffer, 1, bytes_read, f_dest);
    }

    fclose(f_source);
    fclose(f_dest);

    return EXIT_SUCCESS;
}

bool is_parent_dir_valid(char *parent_dir)
{
    struct stat st;
    if (parent_dir != NULL)
    {
        if (lstat(parent_dir, &st) == 0)
        {
            if ((st.st_mode & S_IFMT) == S_IFDIR)
            {
                return true;
            }
        }
    }
    return false;
}

void fail_all_tasks(WorkerArgs *worker_args)
{
    pthread_mutex_lock(get_queue_mutex());
    set_active_tasks(0);
    set_queue_size(0);
    set_done(true);
    pthread_cond_broadcast(get_not_empty_condition());
    pthread_mutex_unlock(get_queue_mutex());
    pthread_mutex_lock(worker_args->mutex);
    worker_args->exit_code = EXIT_FAILURE;
    pthread_mutex_unlock(worker_args->mutex);
}

void *work_copy(void *arg)
{
    DIR *dir;
    Job *job;
    Tuple t = {0};
    struct dirent *d;
    struct stat st_source, st_dest;
    bool dir_name_provided = false;

    WorkerArgs *worker_args = (WorkerArgs *)arg;
    Arena *arena = worker_args->arena;

    while ((job = (Job *)dequeue()) != NULL)
    {
        char *source = job->source;
        char *destination = job->destination;

        if (lstat(source, &st_source) != 0)
        {
            fprintf(stderr, "Error: Invalid source: %s\n", source);
            fail_all_tasks(worker_args);
            return NULL;
        }
        if (lstat(destination, &st_dest) != 0)
        {
            parent_path_from_child(destination, strlen(destination), &t, arena);
            dir_name_provided = is_parent_dir_valid(t.parent);
            if (!dir_name_provided)
            {
                fprintf(stderr, "Error: Invalid destination: %s\n", destination);
                fail_all_tasks(worker_args);
                return NULL;
            }
        }

        bool is_source_dir = (st_source.st_mode & S_IFMT) == S_IFDIR;
        bool is_dest_dir = (st_dest.st_mode & S_IFMT) == S_IFDIR;

        if (is_source_dir && !is_dest_dir && !dir_name_provided)
        {
            fprintf(stderr, "Error: Can't copy directory to a file\n");
            fail_all_tasks(worker_args);
            return NULL;
        }

        if (strcmp(source, destination) == 0)
        {
            fprintf(stderr, "Error: Source and destination are identical\n");
            fail_all_tasks(worker_args);
            return NULL;
        }

        if (!is_source_dir)
        {
            bool is_dest_parent_or_current = check_if_parent_dir(destination);
            if (is_dest_parent_or_current || (is_dest_dir && !is_dest_parent_or_current))
            {
                parent_path_from_child(source, strlen(source), &t, arena);
                if (t.child != NULL)
                {
                    is_dest_dir = false;
                    destination = join_paths(destination, t.child, arena);
                }
            }
            FILE *f_source = fopen(source, "rb");
            FILE *f_dest = is_dest_dir ? fopen(join_paths(destination, source, arena), "wb") : fopen(destination, "wb");
            int success = copy_file(f_source, f_dest);
            if (success != EXIT_SUCCESS)
            {
                fail_all_tasks(worker_args);
                return NULL;
            }
        }
        else
        {
            char *special_dir = parse_special_dir(source, strlen(source));
            if (dir_name_provided)
            {
                int success = mkdir(destination, 0700);
                if (success != 0)
                {
                    fprintf(stderr, "Error copying directory\n");
                    fail_all_tasks(worker_args);
                    return NULL;
                }
            }
            else
            {
                if (special_dir == NULL)
                {
                    destination = !worker_args->flag
                                      ? join_paths(destination, parent_path_from_child(source, strlen(source), &t, arena)->child, arena)
                                      : destination;
                    int success = mkdir(destination, 0700);
                    if (success != 0 && errno != EEXIST)
                    {
                        fprintf(stderr, "Error copying directory\n");
                        fail_all_tasks(worker_args);
                        return NULL;
                    }
                }
            }
            dir = opendir(source);
            if (dir == NULL)
            {
                fprintf(stderr, "Error opening directory\n");
                fail_all_tasks(worker_args);
                return NULL;
            }
            if (!worker_args->flag)
            {
                pthread_mutex_lock(worker_args->mutex);
                worker_args->flag = true;
                pthread_mutex_unlock(worker_args->mutex);
            }
            while ((d = readdir(dir)) != NULL)
            {
                if (!check_if_parent_dir(d->d_name))
                {
                    char *new_source = join_paths(source, d->d_name, arena);
                    char *new_destination = join_paths(destination, d->d_name, arena);
                    if (d->d_type == DT_DIR)
                    {
                        Job *j = (Job *)allocate(arena, sizeof(Job));
                        j->source = new_source;
                        j->destination = new_destination;
                        enqueue(j);
                    }
                    else
                    {
                        FILE *f_source = fopen(new_source, "rb");
                        FILE *f_dest = fopen(new_destination, "wb");
                        int success = copy_file(f_source, f_dest);
                        if (success != EXIT_SUCCESS)
                        {
                            fail_all_tasks(worker_args);
                            return NULL;
                        }
                    }
                }
            }
            closedir(dir);
        }
        pthread_mutex_lock(get_queue_mutex());
        set_active_tasks(get_active_tasks() - 1);
        // If no tasks are left, signal all threads to exit
        if (get_active_tasks() == 0 && get_queue_size() == 0)
        {
            set_done(true);
            pthread_cond_broadcast(get_not_empty_condition()); // Notify all waiting threads
        }
        pthread_mutex_unlock(get_queue_mutex());
    }
    return NULL;
}

int copy_dir_or_file_recursive(char *source, char *destination, bool flag, Arena *arena)
{
    struct stat st_source, st_dest;
    Tuple t = {0};
    struct dirent *d;
    DIR *dir;
    bool dir_name_provided = false;
    if (lstat(source, &st_source) != 0)
    {
        fprintf(stderr, "Error: Invalid source: %s\n", source);
        return EXIT_FAILURE;
    }
    if (lstat(destination, &st_dest) != 0)
    {
        parent_path_from_child(destination, strlen(destination), &t, arena);
        dir_name_provided = is_parent_dir_valid(t.parent);
        if (!dir_name_provided)
        {
            fprintf(stderr, "Error: Invalid destination: %s\n", destination);
            return EXIT_FAILURE;
        }
    }

    bool is_source_dir = (st_source.st_mode & S_IFMT) == S_IFDIR;
    bool is_dest_dir = (st_dest.st_mode & S_IFMT) == S_IFDIR;

    if (is_source_dir && !is_dest_dir && !dir_name_provided)
    {
        fprintf(stderr, "Error: Can't copy directory to a file\n");
        return EXIT_FAILURE;
    }

    if (strcmp(source, destination) == 0)
    {
        fprintf(stderr, "Error: Source and destination are identical\n");
        return EXIT_FAILURE;
    }

    if (!is_source_dir)
    {
        FILE *f_source = fopen(source, "rb");
        FILE *f_dest = is_dest_dir ? fopen(join_paths(destination, source, arena), "wb") : fopen(destination, "wb");
        return copy_file(f_source, f_dest);
    }
    else
    {
        char *special_dir = parse_special_dir(source, strlen(source));
        if (dir_name_provided)
        {
            int success = mkdir(destination, 0700);
            if (success != 0)
            {
                fprintf(stderr, "Error copying directory\n");
                return EXIT_FAILURE;
            }
        }
        else
        {
            if (special_dir == NULL)
            {
                destination = !flag
                                  ? join_paths(destination, parent_path_from_child(source, strlen(source), &t, arena)->child, arena)
                                  : destination;
                int success = mkdir(destination, 0700);
                if (success != 0 && errno != EEXIST)
                {
                    fprintf(stderr, "Error copying directory\n");
                    return EXIT_FAILURE;
                }
            }
        }
        dir = opendir(source);
        if (dir == NULL)
        {
            fprintf(stderr, "Error opening directory\n");
            return EXIT_FAILURE;
        }
        flag = true;
        while ((d = readdir(dir)) != NULL)
        {
            if (!check_if_parent_dir(d->d_name))
            {
                int status = copy_dir_or_file_recursive(join_paths(source, d->d_name, arena), join_paths(destination, d->d_name, arena), flag, arena);
                if (status != EXIT_SUCCESS)
                {
                    return EXIT_FAILURE;
                }
            }
        }
        closedir(dir);
    }

    return EXIT_SUCCESS;
}

int copy_dir_or_file_threaded(char *source, char *destination, Arena *arena)
{
    long number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t threads[number_of_processors];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

    Job j;
    j.source = source;
    j.destination = destination;

    WorkerArgs args = {.arena = arena, .exit_code = 0, .flag = false, .mutex = &mutex};
    enqueue(&j);

    for (int i = 0; i < number_of_processors; i++)
    {
        if (pthread_create(&threads[i], NULL, work_copy, &args) != 0)
        {
            fprintf(stderr, "Failed to create thread %d\n", i);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < number_of_processors; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(get_queue_mutex());
    pthread_cond_destroy(get_not_empty_condition());
    pthread_cond_destroy(get_not_full_condition());

    return args.exit_code;
}

int command_copy(Cli_args *args, Arena *arena)
{
    char *source = args->source;
    char *destination = args->destination;

    return copy_dir_or_file_threaded(source, destination, arena);
    // return copy_dir_or_file_recursive(source, destination, false, arena);
}