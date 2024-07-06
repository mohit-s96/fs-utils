#include <stdio.h>
#include <stdlib.h>
#include "cli.h"

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
