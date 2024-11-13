#include "threads.h"

#define MAX_QUEUE_SIZE 10000

static char *queue[MAX_QUEUE_SIZE];
static int queue_size = 0;
static int active_tasks = 0;
static bool flag = false, done = false;

static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t queue_not_full = PTHREAD_COND_INITIALIZER;

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

// Flag access functions
void set_flag(bool value)
{
    // pthread_mutex_lock(&queue_mutex);
    flag = value;
    // pthread_mutex_unlock(&queue_mutex);
}

bool get_flag()
{
    // pthread_mutex_lock(&queue_mutex);
    bool value = flag;
    // pthread_mutex_unlock(&queue_mutex);
    return value;
}

// Done access functions
void set_done(bool value)
{
    // pthread_mutex_lock(&queue_mutex);
    done = value;
    // pthread_mutex_unlock(&queue_mutex);
}

bool get_done()
{
    // pthread_mutex_lock(&queue_mutex);
    bool value = done;
    // pthread_mutex_unlock(&queue_mutex);
    return value;
}

int get_queue_size()
{
    // pthread_mutex_lock(&queue_mutex);
    int value = queue_size;
    // pthread_mutex_unlock(&queue_mutex);
    return value;
}

void set_queue_size(int value)
{
    // pthread_mutex_lock(&queue_mutex);
    queue_size = value;
    // pthread_mutex_unlock(&queue_mutex);
}

int get_active_tasks()
{
    // pthread_mutex_lock(&queue_mutex);
    int value = active_tasks;
    // pthread_mutex_unlock(&queue_mutex);
    return value;
}

void set_active_tasks(int value)
{
    // pthread_mutex_lock(&queue_mutex);
    active_tasks = value;
    // pthread_mutex_unlock(&queue_mutex);
}

pthread_mutex_t *get_queue_mutex()
{
    return &queue_mutex;
}

pthread_cond_t *get_not_empty_condition()
{
    return &queue_not_empty;
}

pthread_cond_t *get_not_full_condition()
{
    return &queue_not_full;
}