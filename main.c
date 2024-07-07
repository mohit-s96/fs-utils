#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include "cli.h"
#include "colors.c"

int main(int argc, char **argv)
{
    Cli_args *args = parse_cli(argc, argv);
    if (args->command != LS)
        return 1;

    DIR *d;
    struct dirent *dir;
    d = opendir(args->path);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (dir->d_type == DT_DIR)
            {
                print_cyan();
                printf("%s\n", dir->d_name);
                print_reset();
            }
            else if (dir->d_type == DT_LNK)
            {
                print_underlined();
                printf("%s\n", dir->d_name);
                print_reset();
            }
            else if (dir->d_type == DT_UNKNOWN)
            {
                print_red();
                printf("%s\n", dir->d_name);
                print_reset();
            }
            else
            {
                printf("%s\n", dir->d_name);
            }
        }
        closedir(d);
    }
    else
    {
        print_red();
        printf("'%s' is not a valid directory path\n", args->path);
        print_reset();
    }
    free(args);
    return 0;
}
