#pragma once

#include <pthread.h>
#include <stdbool.h>

#define MAX_QUEUE_SIZE 10000
char *queue[MAX_QUEUE_SIZE];
int queue_size = 0;
int active_tasks = 0; // Tracks both queued and dequeued tasks being processed.
bool flag = false, done = false;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_not_full = PTHREAD_COND_INITIALIZER;

#include "threads.h"

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
