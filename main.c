#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "cli.h"
#include "colors.c"
#include "commands.h"
#include "arena.h"

#define ONE_MEG 1024 * 1024

int main(int argc, char **argv)
{
    Arena arena;
    init_arena(&arena, ONE_MEG);
    Cli_args *args = parse_cli(argc, argv, &arena);
    if (args->command != LS)
        return 1;

    command_ls(args, &arena);
    destroy(&arena);
    return 0;
}
