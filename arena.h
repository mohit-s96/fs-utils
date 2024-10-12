#pragma once

#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>

typedef struct
{
    void *start;
    size_t size;
    size_t capacity;
} Arena;

void *init_arena(Arena *arena, size_t size);
void *grow_arena(Arena *arena, size_t size);
void *allocate(Arena *arena, size_t size);
void destroy(Arena *arena);