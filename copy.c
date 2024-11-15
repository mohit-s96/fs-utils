#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include "commands.h"
#include "utils.h"

int copy_file_or_dir(FILE *f_source, FILE *f_dest)
{
    char buffer[BUFSIZ];
    size_t bytes_read;

    if (errno == EACCES)
    {
        fprintf(stderr, "Permission denied\n");
        fclose(f_source);
        fclose(f_dest);
        return EXIT_FAILURE;
    }
    if (f_source == NULL || f_dest == NULL)
    {
        fprintf(stderr, "Error opening file(s)\n");
        fclose(f_source);
        fclose(f_dest);
        return EXIT_FAILURE;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f_source)) > 0)
    {
        fwrite(buffer, 1, bytes_read, f_dest);
    }

    fclose(f_source);
    fclose(f_dest);

    return EXIT_SUCCESS;
}

bool is_parent_dir_valid(char *parent_dir)
{
    struct stat st;
    if (parent_dir != NULL)
    {
        if (lstat(parent_dir, &st) == 0)
        {
            if ((st.st_mode & S_IFMT) == S_IFDIR)
            {
                return true;
            }
        }
    }
    return false;
}

int copy_dir_or_file(char *source, char *destination, bool flag, Arena *arena)
{
    struct stat st_source, st_dest;
    Tuple t = {0};
    struct dirent *d;
    DIR *dir;
    bool dir_name_provided = false;
    if (lstat(source, &st_source) != 0)
    {
        fprintf(stderr, "Error: Invalid source: %s\n", source);
        return EXIT_FAILURE;
    }
    if (lstat(destination, &st_dest) != 0)
    {
        parent_path_from_child(destination, strlen(destination), &t, arena);
        dir_name_provided = is_parent_dir_valid(t.parent);
        if (!dir_name_provided)
        {
            fprintf(stderr, "Error: Invalid destination: %s\n", destination);
            return EXIT_FAILURE;
        }
    }

    bool is_source_dir = (st_source.st_mode & S_IFMT) == S_IFDIR;
    bool is_dest_dir = (st_dest.st_mode & S_IFMT) == S_IFDIR;

    if (is_source_dir && !is_dest_dir && !dir_name_provided)
    {
        fprintf(stderr, "Error: Can't copy directory to a file\n");
        return EXIT_FAILURE;
    }

    if (strcmp(source, destination) == 0)
    {
        fprintf(stderr, "Error: Source and destination are identical\n");
        return EXIT_FAILURE;
    }

    if (!is_source_dir)
    {
        FILE *f_source = fopen(source, "rb");
        FILE *f_dest = is_dest_dir ? fopen(join_paths(destination, source, arena), "wb") : fopen(destination, "wb");
        return copy_file_or_dir(f_source, f_dest);
    }
    else
    {
        /**
         * CASE A: source ends with special directory i.e. "." or ".."
         *
         *         in this case we check if the destination dir name
         *         was provided. if so, we move all the contents to the
         *         destination under the provided dir name. note that
         *         the provided dir name is one which doesn't exist &
         *         needs to be created. if the destination dir provided
         *         already exists, we copy all the contents to the top
         *         level of the pre-existing directory.
         *
         * CASE B: source ends with normal directory name
         *
         *         in this case we check if the dest dir name has been
         *         provided. if it is then we copy the contents from
         *         source to the destination under the provided dir
         *         name. otherwise, the content is copied under the
         *         under a directory with the same name as the source
         *         dir.
         */

        char *special_dir = parse_special_dir(source, strlen(source));
        if (dir_name_provided)
        {
            int success = mkdir(destination, 0700);
            if (success != 0)
            {
                fprintf(stderr, "Error copying directory\n");
                return EXIT_FAILURE;
            }
        }
        else
        {
            if (special_dir == NULL)
            {
                destination = !flag
                                  ? join_paths(destination, parent_path_from_child(source, strlen(source), &t, arena)->child, arena)
                                  : destination;
                int success = mkdir(destination, 0700);
                if (success != 0 && errno != EEXIST)
                {
                    fprintf(stderr, "Error copying directory\n");
                    return EXIT_FAILURE;
                }
            }
        }
        dir = opendir(source);
        flag = true;
        while ((d = readdir(dir)) != NULL)
        {
            if (!check_if_parent_dir(d->d_name))
            {
                if (strcmp(d->d_name, ".git") == 0)
                {
                    (void)1;
                }
                int status = copy_dir_or_file(join_paths(source, d->d_name, arena), join_paths(destination, d->d_name, arena), flag, arena);
                if (status != EXIT_SUCCESS)
                {
                    return EXIT_FAILURE;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

int command_copy(Cli_args *args, Arena *arena)
{
    char *source = args->source;
    char *destination = args->destination;

    return copy_dir_or_file(source, destination, false, arena);
}