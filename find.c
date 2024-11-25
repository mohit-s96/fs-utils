#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include "commands.h"
#include "utils.h"
#include "colors.h"
#include "pattern.h"
#include "threads.h"

typedef struct
{
    char *search_pattern;
    Arena *arena;
    int id;
    bool no_recurse;
    int is_stdout;
} WorkerArgs;

void *work(void *arg)
{
    WorkerArgs *worker_args = (WorkerArgs *)arg;
    char *path;
    while ((path = (char *)dequeue()) != NULL)
    {
        struct dirent *d;
        DIR *dir;
        dir = opendir(path);
        if (dir == NULL)
        {
            pthread_mutex_lock(get_queue_mutex());
            set_active_tasks(get_active_tasks() - 1); // Decrement active tasks if directory open fails
            pthread_mutex_unlock(get_queue_mutex());
            continue;
        }
        while ((d = readdir(dir)) != NULL)
        {
            bool is_dir = d->d_type == DT_DIR;
            char *dir_name = d->d_name;
            if (match_pattern(worker_args->search_pattern, dir_name))
            {
                set_flag(true);
                if (is_dir)
                {
                    print_cyan(worker_args->is_stdout);
                }
                printf("%s/%s\n", path, dir_name);
                if (is_dir)
                {
                    print_reset(worker_args->is_stdout);
                }
            }
            if (is_dir && !worker_args->no_recurse && !check_if_parent_dir(dir_name))
            {
                char *new_path = join_paths(path, dir_name, worker_args->arena);
                enqueue(new_path);
            }
        }
        closedir(dir);
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

int command_find(Cli_args *args, Arena *arena)
{
    long number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);

    pthread_t threads[number_of_processors];

    struct stat input_dir_stat;
    if (lstat(args->path, &input_dir_stat) == -1)
    {
        perror("stat");
        return EXIT_FAILURE;
    }
    if (!S_ISDIR(input_dir_stat.st_mode))
    {
        fprintf(stderr, "Not a directory\n");
        return EXIT_FAILURE;
    }
    if (!args->search_pattern)
    {
        fprintf(stderr, "No search pattern provided\n");
        return EXIT_FAILURE;
    }

    int should_print_color = isatty(fileno(stdout));

    enqueue(args->path);

    WorkerArgs worker_args = {.arena = arena, .search_pattern = args->search_pattern, .no_recurse = args->no_recurse, .is_stdout = should_print_color};
    worker_args.arena = arena;
    worker_args.search_pattern = args->search_pattern;
    for (int i = 0; i < number_of_processors; i++)
    {
        worker_args.id = i;
        if (pthread_create(&threads[i], NULL, work, &worker_args) != 0)
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

    if (!get_flag())
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
