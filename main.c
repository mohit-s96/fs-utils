#include <stdio.h>
#include <stdbool.h>

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

int main(int argc, char **argv)
{
    printf("%s\n", "init");
    return 0;
}