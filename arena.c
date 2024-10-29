#include <pthread.h>
#include "arena.h"

void *init_arena(Arena *arena, size_t size)
{
    void *start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (start == MAP_FAILED)
    {
        return NULL;
    }
    Block *head_block = (Block *)malloc(sizeof(Block));
    head_block->capacity = size;
    head_block->next = NULL;
    head_block->size = 0;
    head_block->start = start;

    arena->head = head_block;
    arena->num_blocks = 1;
    arena->tail = NULL;

    // Initialize the mutex for this arena
    pthread_mutex_init(&arena->mutex, NULL);

    return NULL;
}

void grow_arena(Arena *arena, size_t size)
{
    void *start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (start == MAP_FAILED)
    {
        return;
    }
    Block *new_block = (Block *)malloc(sizeof(Block));
    new_block->capacity = size;
    new_block->next = NULL;
    new_block->size = 0;
    new_block->start = start;

    arena->num_blocks++;
    if (arena->tail != NULL)
    {
        arena->tail->next = new_block;
    }
    else
    {
        arena->head->next = new_block;
    }
    arena->tail = new_block;
}

void *allocate(Arena *arena, size_t size)
{
    pthread_mutex_lock(&arena->mutex);
    Block *block = arena->head;
    unsigned long long total_block_size = 0;
    while (block != NULL)
    {
        if (block->size + size < block->capacity)
        {
            void *free_space = (char *)block->start + block->size;
            block->size += size;
            pthread_mutex_unlock(&arena->mutex);
            return free_space;
        }
        total_block_size += block->size;
        block = block->next;
    }

    // reaching here means needs more memory

    // double the size
    unsigned long long average_block_size = total_block_size / arena->num_blocks;
    grow_arena(arena, (average_block_size + size) * 2);
    arena->tail->size += size;

    pthread_mutex_unlock(&arena->mutex);

    return arena->tail->start;
}

void destroy(Arena *arena)
{
    pthread_mutex_lock(&arena->mutex);
    Block *block = arena->head;
    while (block != NULL)
    {
        int success = munmap(block->start, block->capacity);

        if (success != 0)
        {
            fprintf(stderr, "Couldn't unmap the arena");
        }

        Block *next = block->next;
        free(block);
        block = next;
    }

    pthread_mutex_unlock(&arena->mutex);
    pthread_mutex_destroy(&arena->mutex);
}
