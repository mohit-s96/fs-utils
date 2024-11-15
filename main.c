#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "cli.h"
#include "commands.h"
#include "arena.h"

#define ONE_MEG 1024 * 1024

int main(int argc, char **argv)
{
    Arena arena;
    init_arena(&arena, ONE_MEG);
    Cli_args *args = parse_cli(argc, argv, &arena);
    int exit_code = EXIT_SUCCESS;
    if (args->command == LS)
        exit_code = command_ls(args, &arena);
    else if (args->command == F)
        exit_code = command_find(args, &arena);
    else if (args->command == NEW)
        exit_code = command_new(args, &arena);
    else if (args->command == SIZE)
        exit_code = command_size(args, &arena);
    else if (args->command == CP)
        exit_code = command_copy(args, &arena);
    else
    {
        exit_code = EXIT_FAILURE;
    }
    destroy(&arena);
    return exit_code;
}
