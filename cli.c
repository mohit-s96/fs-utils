#include "cli.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef int (*cli_parser)(Cli_args *args, int argc, char **argv, int start_at);

typedef struct
{
    char *name;
    commands command;
    cli_parser parser;
} command_lookup;

int parse_ls(Cli_args *, int, char **, int);
int parse_find(Cli_args *, int, char **, int);
int parse_cp_mv(Cli_args *, int, char **, int);
int parse_new(Cli_args *, int, char **, int);
int parse_stat(Cli_args *, int, char **, int);

command_lookup command_table[] = {
    {"ls", LS, parse_ls},
    {"f", F, parse_find},
    {"cp", CP, parse_cp_mv},
    {"mv", MV, parse_cp_mv},
    {"new", NEW, parse_new},
    {"stat", STAT, parse_stat},
    {NULL, UNSUPPORTED_COMMAND, NULL}};

commands get_command_type(char *command)
{
    for (int i = 0; command_table[i].name != NULL; i++)
    {
        if (0 == strcmp(command_table[i].name, command))
        {
            return command_table[i].command;
        }
    }
    return UNSUPPORTED_COMMAND;
}

void parse_options(Cli_args *args, int argc, char **argv, int *start_at)
{
    while (*start_at < argc)
    {
        char *input = argv[*start_at];

        if (0 == strcmp(input, "-s"))
        {
            args->sort_by_size = true;
        }
        if (0 == strcmp(input, "-a"))
        {
            args->sort_by_name = true;
        }
        if (0 == strcmp(input, "-nr"))
        {
            args->no_recurse = true;
        }
        if (0 == strcmp(input, "--file"))
        {
            if (*start_at + 1 < argc)
            {
                args->search_pattern = argv[(*start_at) + 1];
            }
        }
        if (0 == strcmp(input, "-d"))
        {
            if (*start_at + 1 < argc)
            {
                args->new_dir_name = argv[(*start_at) + 1];
            }
        }

        (*start_at)++;
    }
}

int parse_ls(Cli_args *args, int argc, char **argv, int start_at)
{
    if (start_at < argc)
    {
        if (argv[start_at][0] != '-')
        {
            args->path = argv[start_at];
        }
    }
    parse_options(args, argc, argv, &start_at);
    if (!args->path)
        args->path = ".";
    return 0;
}

int parse_find(Cli_args *args, int argc, char **argv, int start_at)
{
    if (start_at < argc)
    {
        args->path = argv[start_at];
    }
    parse_options(args, argc, argv, &start_at);
    return 0;
}

int parse_cp_mv(Cli_args *args, int argc, char **argv, int start_at)
{
    if (start_at < argc)
    {
        args->source = argv[start_at++];
    }
    if (start_at < argc)
    {
        args->destination = argv[start_at];
    }
    return 0;
}

int parse_new(Cli_args *args, int argc, char **argv, int start_at)
{
    int start = start_at;
    parse_options(args, argc, argv, &start_at);
    if (start < argc)
    {
        if (!args->new_dir_name)
        {
            args->new_file_name = argv[start];
        }
    }
    return 0;
}

int parse_stat(Cli_args *args, int argc, char **argv, int start_at)
{
    if (start_at < argc)
    {
        args->path = argv[start_at];
    }
    parse_options(args, argc, argv, &start_at);
    return 0;
    return 0;
}

Cli_args *parse_cli(int argc, char **argv)
{
    char *subcommand = "";
    char *path = "";
    Cli_args *args = (void *)malloc(sizeof(Cli_args));

    if (argc == 1)
    {
        args->command = LS;
        args->path = ".";
        return args;
    }

    commands command = get_command_type(argv[1]);
    args->command = command == UNSUPPORTED_COMMAND ? LS : command;

    int start_at = command == UNSUPPORTED_COMMAND ? 1 : 2;

    for (int i = 0; command_table[i].name != NULL; i++)
    {
        if (command_table[i].command == args->command)
        {
            command_table[i].parser(args, argc, argv, start_at);
            break;
        }
    }

    return args;
}