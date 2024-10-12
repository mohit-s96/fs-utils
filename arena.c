#include "arena.h"

void *init_arena(Arena *arena, size_t size)
{
    void *start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (start == MAP_FAILED)
    {
        return NULL;
    }
    arena->start = start;
    arena->size = 0;
    arena->capacity = size;

    return start;
}

void *grow_arena(Arena *arena, size_t size)
{
    if (size <= arena->capacity)
    {
        return arena->start;
    }

    void *start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (start == MAP_FAILED)
    {
        return NULL;
    }

    memcpy(start, arena->start, arena->size);
    int success = munmap(arena->start, arena->capacity);
    if (success != 0)
    {
        fprintf(stderr, "Couldn't unmap the arena");
        return NULL;
    }

    arena->start = start;
    arena->capacity = size;

    return start;
}

void *allocate(Arena *arena, size_t size)
{
    if (arena->size + size > arena->capacity)
    {
        // double
        return grow_arena(arena, arena->capacity * 2);
    }

    void *address = (char *)arena->start + arena->size;
    arena->size += size;

    return address;
}

void destroy(Arena *arena)
{
    int success = munmap(arena->start, arena->capacity);
    if (success != 0)
    {
        fprintf(stderr, "Couldn't unmap the arena");
    }
}