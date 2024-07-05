#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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
    ls,
    f,
    cp,
    mv,
    new,
    stat,
    UNSUPPORTED_COMMAND
} commands;

typedef struct Cli_args
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
        return new;
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
        if (command == new && !args->new_dir_name)
        {
            // next entry should be a a file name
            args->new_dir_name = input;
        }
        if (command == ls && !args->path)
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
                args->search_pattern = argv[i++];
            }
        }
        if (0 == strcasecmp(input, "-d"))
        {
            if (i + 1 < argc)
            {
                args->new_dir_name = argv[i++];
            }
        }
    }

    if (command == ls && !args->path)
    {
        args->path = ".";
    }

    return args;
}

int main(int argc, char **argv)
{
    Cli_args *args = parse_cli(argc, argv);
    printf("command => %d\n", args->command);
    printf("path => %s\n", args->path);
    printf("destination => %s\n", args->destination);
    printf("source => %s\n", args->source);
    printf("-s => %d\n", args->sort_by_size);
    printf("-a => %d\n", args->sort_by_name);
    printf("new dir => %s\n", args->new_dir_name);
    printf("new file => %s\n", args->new_file_name);
    printf("-nr => %d\n", args->no_recurse);
    printf("search pattern => %s\n", args->search_pattern);

    free(args);
    return 0;
}