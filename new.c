#include "commands.h"
#include "utils.h"

int command_new(Cli_args *args, Arena *arena)
{
    if (args->new_dir_name)
    {
        int success = mkdir(join_paths(".", args->new_dir_name, arena), 0700);
        if (success != 0)
        {
            fprintf(stderr, "Error creating directory\n");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    if (args->new_file_name)
    {
        FILE *file = fopen(join_paths(".", args->new_file_name, arena), "wx");
        if (file == NULL)
        {
            fprintf(stderr, "Error creating file\n");
            return EXIT_FAILURE;
        }
        fclose(file);
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}