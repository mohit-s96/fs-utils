#include "threads.h"

#define MAX_QUEUE_SIZE 10000

static void *queue[MAX_QUEUE_SIZE]; // Queue now stores `void *`
static int queue_size = 0;
static int active_tasks = 0;
static bool flag = false, done = false;

static pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t queue_not_full = PTHREAD_COND_INITIALIZER;

void enqueue(void *item)
{
    pthread_mutex_lock(&queue_mutex);
    while (queue_size == MAX_QUEUE_SIZE)
    {
        pthread_cond_wait(&queue_not_full, &queue_mutex);
    }
    queue[queue_size++] = item;
    active_tasks++;
    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&queue_mutex);
}

void *dequeue()
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
    void *item = queue[--queue_size];
    queue[queue_size] = NULL;
    pthread_cond_signal(&queue_not_full);
    pthread_mutex_unlock(&queue_mutex);

    return item;
}

// Flag and done setters and getters
void set_flag(bool value)
{
    flag = value;
}

bool get_flag()
{
    return flag;
}

void set_done(bool value)
{
    done = value;
}

bool get_done()
{
    return done;
}

// Queue size and active task management
int get_queue_size()
{
    return queue_size;
}

void set_queue_size(int value)
{
    queue_size = value;
}

int get_active_tasks()
{
    return active_tasks;
}

void set_active_tasks(int value)
{
    active_tasks = value;
}

// Synchronization primitives
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
