#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include "utils.h"
#include "cli.h"
#include "colors.h"

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

int cmp_file_size(const void *a, const void *b)
{
    return ((FileStats *)b)->size - ((FileStats *)a)->size;
}

int cmp_file_name(const void *a, const void *b)
{
    return strcasecmp(((FileStats *)a)->name, ((FileStats *)b)->name);
}

int cmp_file_date(const void *a, const void *b)
{
    return ((FileStats *)b)->last_modified - ((FileStats *)a)->last_modified;
}

void print_name(FileStats *stats)
{
    switch (stats->mode & S_IFMT)
    {
    case S_IFBLK:
    case S_IFCHR:
    case S_IFIFO:
        print_bold();
        printf("%-30s", stats->name);
        print_reset();
        break;
    case S_IFDIR:
        print_cyan();
        printf("%-30s", stats->name);
        print_reset();
        break;
    case S_IFLNK:
        print_blue();
        printf("%-30s", stats->name);
        print_reset();
        break;
    case S_IFREG:
        printf("%-30s", stats->name);
        break;
    default:
        printf("%-30s", stats->name);
        break;
    }
}

void print_stats(FileStats *stats, unsigned int size)
{
    printf("Total %d\n", size);
    char buffer[10];
    char s[100];
    for (int i = 0; i < size; i++)
    {
        char *uname = get_user_name_from_uid(stats[i].uid);
        char *gname = get_group_name_from_gid(stats[i].gid);
        format_size(stats[i].size, buffer, sizeof(buffer));
        struct tm *p = localtime(&stats[i].last_modified);
        strftime(s, sizeof s, "%a %b %d %Y %H:%M", p);

        printf("%-6s %-8s %-8s %-10s %-25s", stats[i].permissions, uname, gname, buffer, s);

        print_name(&stats[i]);
        printf("\n");
    }
}

int get_dir_item_count(char *path)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    int i = 0;
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            i++;
        }
        closedir(d);
    }
    return i;
}

int command_ls(Cli_args *args, Arena *arena)
{
    char *path = args->path;
    bool sort_by_size = args->sort_by_size;
    bool sort_by_name = args->sort_by_name;

    struct stat sb;
    if (lstat(path, &sb) == -1)
    {
        perror("stat");
        return EXIT_FAILURE;
    }
    // handle case when ls is called on not-a-dir
    if ((sb.st_mode & S_IFMT) != S_IFDIR)
    {
        FileStats *stat_list = (FileStats *)allocate(arena, sizeof(FileStats));
        stat_list->gid = sb.st_gid;
        stat_list->uid = sb.st_uid;
        stat_list->size = sb.st_size;
        stat_list->inode = sb.st_ino;
        stat_list->mode = sb.st_mode;
        stat_list->name = path;
        stat_list->permissions = get_user_permissions(sb.st_mode, arena);

        print_stats(stat_list, 1);
        return EXIT_SUCCESS;
    }

    int i = 0;
    int num_items = get_dir_item_count(path);
    FileStats *stat_list = (FileStats *)allocate(arena, num_items * sizeof(FileStats));

    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            stat_list[i].name = duplicate_string(dir->d_name, arena);
            stat_list[i].inode = dir->d_ino;

            char *full_path = join_paths(path, dir->d_name, arena);
            if (lstat(full_path, &sb) == -1)
            {
                perror("stat");
                return EXIT_FAILURE;
            }

            stat_list[i].permissions = get_user_permissions(sb.st_mode, arena);
            stat_list[i].uid = sb.st_uid;
            stat_list[i].gid = sb.st_gid;
            stat_list[i].mode = sb.st_mode;
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
            stat_list[i].last_modified = sb.st_mtimespec.tv_sec;
#else
            stat_list[i].last_modified = sb.st_mtime;
#endif
            if ((sb.st_mode & S_IFMT) == S_IFDIR && !check_if_parent_dir(dir->d_name))
            {
                unsigned long long s = get_dir_size(full_path, args->depth);
                stat_list[i].size = s;
            }
            else
            {
                stat_list[i].size = sb.st_size;
            }
            i++;
        }
        closedir(d);
    }
    if (sort_by_size)
    {
        quick_sort(stat_list, sizeof(FileStats), 0, num_items - 1, cmp_file_size);
    }
    else if (sort_by_name)
    {
        quick_sort(stat_list, sizeof(FileStats), 0, num_items - 1, cmp_file_name);
    }
    else
    {
        quick_sort(stat_list, sizeof(FileStats), 0, num_items - 1, cmp_file_date);
    }
    print_stats(stat_list, num_items);
    return EXIT_SUCCESS;
}