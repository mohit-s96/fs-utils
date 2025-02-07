#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "arena.h"

typedef enum
{
    LS,
    F,
    CP,
    MV,
    NEW,
    SIZE,
    UNSUPPORTED_COMMAND
} commands;

#define MAX_CLI_ARGS 1000

typedef struct
{
    int depth;
    uint32_t num_dir_or_files;
    commands command;
    char *search_pattern;
    char *source;
    char *destination;
    char *new_file_name[MAX_CLI_ARGS];
    char *new_dir_name[MAX_CLI_ARGS];
    char *path;
    bool sort_by_size;
    bool sort_by_name;
    bool no_recurse;
} Cli_args;

commands get_command_type(char *command);

Cli_args *parse_cli(int argc, char **argv, Arena *arena);