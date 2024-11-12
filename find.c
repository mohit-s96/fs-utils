#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "commands.h"
#include "utils.h"
#include "colors.h"
#include "pattern.h"

#define MAX_QUEUE_SIZE 10000
char *queue[MAX_QUEUE_SIZE];
int queue_size = 0;
int active_tasks = 0; // Tracks both queued and dequeued tasks being processed.
bool flag = false, done = false;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_not_full = PTHREAD_COND_INITIALIZER;

typedef struct
{
    char *search_pattern;
    Arena *arena;
    int id;
    bool no_recurse;
} WorkerArgs;

void enqueue(char *path)
{
    pthread_mutex_lock(&queue_mutex);
    while (queue_size == MAX_QUEUE_SIZE)
    {
        pthread_cond_wait(&queue_not_full, &queue_mutex);
    }
    queue[queue_size++] = path;
    active_tasks++;
    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&queue_mutex);
}

char *dequeue()
{
    pthread_mutex_lock(&queue_mutex);
    while (queue_size == 0 && !done)
    {
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }
    if (done && queue_size == 0)
    {
        pthread_mutex_unlock(&queue_mutex);
        return NULL;
    }
    char *path = queue[--queue_size];
    queue[queue_size] = NULL;
    pthread_cond_signal(&queue_not_full);
    pthread_mutex_unlock(&queue_mutex);

    return path;
}

void *work(void *arg)
{
    WorkerArgs *worker_args = (WorkerArgs *)arg;
    char *path;
    // int id = worker_args->id;
    while ((path = dequeue()) != NULL)
    {
        struct dirent *d;
        DIR *dir;
        dir = opendir(path);
        if (dir == NULL)
        {
            pthread_mutex_lock(&queue_mutex);
            active_tasks--; // Decrement active tasks if directory open fails
            pthread_mutex_unlock(&queue_mutex);
            continue;
        }
        // printf("thread [%d] - processing: %s\n", id, path);
        while ((d = readdir(dir)) != NULL)
        {
            bool is_dir = d->d_type == DT_DIR;
            char *dir_name = d->d_name;
            if (match_pattern(worker_args->search_pattern, dir_name))
            {
                flag = true;
                if (is_dir)
                {
                    print_cyan();
                }
                printf("%s/%s\n", path, dir_name);
                if (is_dir)
                {
                    print_reset();
                }
            }
            if (is_dir && !worker_args->no_recurse && !check_if_parent_dir(dir_name))
            {
                char *new_path = join_paths(path, dir_name, worker_args->arena);
                // printf("thread [%d] - new path: %s\n", id, new_path);
                enqueue(new_path);
                // printf("thread [%d] - enqueued: %s\n", id, new_path);
            }
        }
        closedir(dir);
        // printf("thread [%d] - done with: %s\n", id, path);
        pthread_mutex_lock(&queue_mutex);
        active_tasks--;

        // If no tasks are left, signal all threads to exit
        if (active_tasks == 0 && queue_size == 0)
        {
            done = true;
            pthread_cond_broadcast(&queue_not_empty); // Notify all waiting threads
        }
        pthread_mutex_unlock(&queue_mutex);
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

    queue[queue_size++] = args->path;
    active_tasks++;

    WorkerArgs worker_args = {.arena = arena, .search_pattern = args->search_pattern, .no_recurse = args->no_recurse};
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

    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&queue_not_empty);
    pthread_cond_destroy(&queue_not_full);

    if (!flag)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
