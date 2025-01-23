#include "commands.h"
#include "utils.h"

int command_new(Cli_args *args, Arena *arena)
{
    bool is_dir_command = args->new_dir_name[0] != NULL;
    bool is_file_command = args->new_file_name[0] != NULL;
    int success;
    if (is_dir_command)
    {
        for (int i = 0; i < args->num_dir_or_files; i++)
        {
            success = mkdir(args->new_dir_name[i], 0700);
            if (success != 0)
            {
                fprintf(stderr, "Error creating directory: %s\n", args->new_dir_name[i]);
                return EXIT_FAILURE;
            }
        }
        return EXIT_SUCCESS;
    }
    if (is_file_command)
    {

        for (int i = 0; i < args->num_dir_or_files; i++)
        {

            FILE *file = fopen(args->new_file_name[i], "wx");
            if (file == NULL)
            {
                fprintf(stderr, "Error creating file: %s\n", args->new_file_name[i]);
                return EXIT_FAILURE;
            }
            fclose(file);
        }
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}