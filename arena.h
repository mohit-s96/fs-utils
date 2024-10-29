#pragma once

#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

typedef struct Block
{
    void *start;
    size_t size;
    size_t capacity;
    struct Block *next;
} Block;

typedef struct
{
    Block *head;
    Block *tail;
    int num_blocks;
    pthread_mutex_t mutex;
} Arena;

void *init_arena(Arena *arena, size_t size);
void grow_arena(Arena *arena, size_t size);
void *allocate(Arena *arena, size_t size);
void destroy(Arena *arena);