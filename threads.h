#pragma once
#include <stdbool.h>
#include <pthread.h>

// Generic enqueue and dequeue functions
void enqueue(void *item);
void *dequeue();

// Configuration for flag and done status
void set_flag(bool value);
bool get_flag();

void set_done(bool value);
bool get_done();

// Queue size and active task management
int get_queue_size();
void set_queue_size(int value);

int get_active_tasks();
void set_active_tasks(int value);

// Synchronization primitives
pthread_mutex_t *get_queue_mutex();
pthread_cond_t *get_not_full_condition();
pthread_cond_t *get_not_empty_condition();
