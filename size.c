#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include "commands.h"
#include "utils.h"

#define MAX_DEPTH 100

int command_size(Cli_args *args, Arena *arena)
{
    char *path = args->path;
    struct stat sb;
    char buffer[10];
    if (lstat(path, &sb) == -1)
    {
        perror("stat");
        return EXIT_FAILURE;
    }
    if ((sb.st_mode & S_IFMT) != S_IFDIR)
    {
        format_size(sb.st_size, buffer, sizeof(buffer));
        printf("%s\n", buffer);
    }
    else
    {
        unsigned long long recursive_dir_size = get_dir_size(path, args->depth > 1 ? args->depth : MAX_DEPTH);
        format_size(recursive_dir_size, buffer, sizeof(buffer));
        printf("%s\n", buffer);
    }
    return EXIT_SUCCESS;
}