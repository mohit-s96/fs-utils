#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include "utils.h"
#include "cli.h"
#include <time.h>
#include <string.h>

typedef struct
{
    char *name;
    char *permissions;
    ino_t inode;
    unsigned long long size;
    mode_t mode;
    uid_t uid;
    gid_t gid;
    long last_modified;
} FileStats;

void command_ls(Cli_args *args, Arena *arena)
{
    char *path = args->path;
    // bool sort_by_size = args->sort_by_size;
    // bool sort_by_name = args->sort_by_name;

    // stat the input path to get number of hard links
    struct stat sb;
    if (stat(path, &sb) == -1)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    nlink_t links = sb.st_nlink;

    // alloc `links` number of nodes
    int i = 0;
    FileStats *stat_list = (FileStats *)allocate(arena, links * sizeof(FileStats));

    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    if (d)
    {
        while ((dir = readdir(d)) != NULL && i < links)
        {
            stat_list[i].name = dir->d_name;
            stat_list[i].inode = dir->d_ino;

            char *full_path = join_paths(path, dir->d_name, arena);
            if (stat(full_path, &sb) == -1)
            {
                perror("stat");
                exit(EXIT_FAILURE);
            }

            stat_list[i].permissions = get_user_permissions(sb.st_mode, arena);
            stat_list[i].uid = sb.st_uid;
            stat_list[i].gid = sb.st_gid;
            stat_list[i].mode = sb.st_mode;
            stat_list[i].last_modified = sb.st_mtimespec.tv_nsec;
            if ((sb.st_mode & S_IFMT) == S_IFDIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
            {
                stat_list[i].size = get_dir_size(full_path, 1);
            }
            else
            {
                stat_list[i].size = sb.st_size;
            }
            i++;
        }
    }

    for (int j = 0; j < links; j++)
    {
        printf("Name:             %s\n", stat_list[j].name);
        printf("Permisssions:     %s\n", stat_list[j].permissions);
        printf("UID:              %d\n", stat_list[j].uid);
        printf("GID:              %d\n", stat_list[j].gid);
        printf("Mode:             %lo\n", (unsigned long)stat_list[j].mode);
        printf("Last Modified:    %s\n", ctime(&stat_list[j].last_modified));
        printf("Size:             %lld\n", stat_list[j].size);

        printf("------------------------------------\n");
    }
    closedir(d);
}