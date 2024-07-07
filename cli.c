#include "cli.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

commands get_command_type(char *command)
{
    if (0 == strcmp(command, "ls"))
    {
        return ls;
    }
    if (0 == strcmp(command, "f"))
    {
        return f;
    }
    if (0 == strcmp(command, "cp"))
    {
        return cp;
    }
    if (0 == strcmp(command, "mv"))
    {
        return mv;
    }
    if (0 == strcmp(command, "new"))
    {
        return neww;
    }
    if (0 == strcmp(command, "stat"))
    {
        return stat;
    }
    return UNSUPPORTED_COMMAND;
}

Cli_args *parse_cli(int argc, char **argv)
{
    char *subcommand = "";
    char *path = "";
    Cli_args *args = (void *)malloc(sizeof(Cli_args));
    // some defaults
    if (argc == 1)
    {
        args->command = ls;
        args->path = ".";
        return args;
    }

    commands command = get_command_type(argv[1]);
    args->command = command == UNSUPPORTED_COMMAND ? ls : command;

    int i = command == UNSUPPORTED_COMMAND ? 1 : 2;

    for (; i < argc; i++)
    {
        char *input = argv[i];
        if (command == cp || command == mv)
        {
            args->source = input;
            if (i + 1 < argc)
            {
                args->destination = argv[i + 1];
            }
            break;
        }
        if ((command == f || command == stat) && !args->path)
        {
            // next entry should be a path
            args->path = input;
        }
        if (command == neww && !args->new_dir_name)
        {
            // next entry should be a file name
            if (input[0] != '-')
            {

                args->new_file_name = input;
            }
        }
        if (args->command == ls && !args->path)
        {
            // next entry should be a a path
            if (input[0] != '-')
            {
                args->path = input;
            }
        }

        if (0 == strcasecmp(input, "-s"))
        {
            args->sort_by_size = true;
        }
        if (0 == strcasecmp(input, "-a"))
        {
            args->sort_by_name = true;
        }
        if (0 == strcasecmp(input, "-nr"))
        {
            args->no_recurse = true;
        }
        if (0 == strcasecmp(input, "--file"))
        {
            if (i + 1 < argc)
            {
                args->search_pattern = argv[++i];
            }
        }
        if (0 == strcasecmp(input, "-d"))
        {
            if (i + 1 < argc)
            {
                args->new_dir_name = argv[++i];
            }
        }
    }

    if (args->command == ls && !args->path)
    {
        args->path = ".";
    }

    return args;
}