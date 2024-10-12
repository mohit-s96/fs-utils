#pragma once

#include <stdbool.h>
#include "arena.h"

typedef struct File
{
    char *name;
    char *contents;
    bool isDir;
    bool isFile;
    struct File **children;
    struct File *sibling;
} File;

typedef enum
{
    LS,
    F,
    CP,
    MV,
    NEW,
    STAT,
    UNSUPPORTED_COMMAND
} commands;

typedef struct
{
    commands command;
    bool sort_by_size;
    bool sort_by_name;
    bool no_recurse;
    char *search_pattern;
    char *source;
    char *destination;
    char *new_file_name;
    char *new_dir_name;
    char *path;
} Cli_args;

commands get_command_type(char *command);

Cli_args *parse_cli(int argc, char **argv, Arena *arena);