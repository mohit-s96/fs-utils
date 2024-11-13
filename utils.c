#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <limits.h>

#include "utils.h"

void swap(void *a, void *b, int size)
{
    char *temp = (char *)malloc(size);
    memcpy(temp, a, size);
    memcpy(a, b, size);
    memcpy(b, temp, size);
    free(temp);
}

int partition(void *arr, size_t size, int low, int high, int (*cmp)(const void *, const void *))
{
    char *array = (char *)arr;
    // last element pivot
    void *pivot = array + size * high;

    int i = low - 1;

    for (int j = low; j < high; j++)
    {
        if (cmp(array + j * size, pivot) <= 0)
        {
            i++;
            swap(array + i * size, array + j * size, size);
        }
    }
    i++;
    swap(array + i * size, pivot, size);
    return i;
}

void quick_sort(void *arr, size_t size, int low, int high, int (*cmp)(const void *, const void *))
{
    if (low < high)
    {
        int pivot = partition(arr, size, low, high, cmp);
        quick_sort(arr, size, low, pivot - 1, cmp);
        quick_sort(arr, size, pivot + 1, high, cmp);
    }
}

int cmp_int(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

int cmp_char(const void *a, const void *b)
{
    return (*(char *)a - *(char *)b);
}

int cmp_string(const void *a, const void *b)
{
    return strcmp(*(char **)a, *(char **)b);
}

bool is_symlink(mode_t mode)
{
    return (mode & S_IFMT) == S_IFLNK;
}

bool check_if_parent_dir(char *dir_name)
{
    return strcmp(dir_name, ".") == 0 || strcmp(dir_name, "..") == 0;
}

long long get_dir_size(const char *path, int max_depth)
{
    DIR *dir;
    struct dirent *entry;
    char full_path[1024];
    struct stat st;
    long long total_size = 0;

    if (max_depth < 0)
    {
        return 0;
    }

    dir = opendir(path);
    if (dir == NULL)
    {
        return 0;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (check_if_parent_dir(entry->d_name))
        {
            continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        if (lstat(full_path, &st) == 0)
        {
            if (S_ISDIR(st.st_mode))
            {
                total_size += get_dir_size(full_path, max_depth - 1);
            }
            else
            {
                // block size of 512 bytes
                total_size += st.st_blocks * 512;
            }
        }
    }

    closedir(dir);
    return total_size;
}

char *get_user_name_from_uid(uid_t uid)
{
    struct passwd *pwd;
    pwd = getpwuid(uid);
    if (pwd != NULL)
    {
        return pwd->pw_name;
    }
    return NULL;
}

char *get_group_name_from_gid(gid_t gid)
{
    struct group *grp;
    grp = getgrgid(gid);
    if (grp != NULL)
    {
        return grp->gr_name;
    }
    return NULL;
}

char *join_paths(const char *path1, const char *path2, Arena *arena)
{
    // Allocate enough space for the new path (with possible slash and null terminator)
    size_t len1 = strlen(path1);
    size_t len2 = strlen(path2);
    char *result = allocate(arena, len1 + len2 + 2); // 1 for potential '/' + 1 for '\0'

    if (result == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Copy the first path
    strcpy(result, path1);

    // Add a '/' if path1 doesn't end with it
    if (len1 > 0 && result[len1 - 1] != '/')
    {
        strcat(result, "/");
    }

    // Append the second path
    strcat(result, path2);

    return result;
}

char *get_user_permissions(mode_t mode, Arena *arena)
{
    // Allocate a string for the permission format "rwx" + null terminator
    char *permissions = (char *)allocate(arena, 4); // "rwx" is 3 characters, +1 for '\0'

    if (permissions == NULL)
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Set permissions for user (owner) - UID
    permissions[0] = (mode & S_IRUSR) ? 'r' : '-'; // Read permission
    permissions[1] = (mode & S_IWUSR) ? 'w' : '-'; // Write permission
    permissions[2] = (mode & S_IXUSR) ? 'x' : '-'; // Execute permission
    permissions[3] = '\0';                         // Null terminator for the string

    return permissions;
}

void format_size(unsigned long long bytes, char *output, size_t output_size)
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int unit_index = 0;
    double size = (double)bytes;

    // Scale the size down to a more suitable unit
    while (size >= 1024 && unit_index < 5)
    {
        size /= 1024;
        unit_index++;
    }

    // Format the result into the output buffer
    snprintf(output, output_size, "%.1f%s", size, units[unit_index]);
}

int safe_parse_cli_int(char *num)
{
    char *endptr;
    errno = 0; // Reset errno before the call to strtol

    long value = strtol(num, &endptr, 10);

    if (errno == ERANGE)
    {
        // Error: The number is out of range for a long integer
        return 1;
    }
    if (endptr == num)
    {
        // Error: No digits were found. Invalid input
        return 1;
    }
    if (*endptr != '\0')
    {
        // Error: Trailing characters after the number
        return 1;
    }
    if (value < INT_MIN || value > INT_MAX)
    {
        // Error: The number is out of range for an int
        return 1;
    }

    // Safe to cast to int
    return (int)value;
}

char *duplicate_string(char *str, Arena *arena)
{
    int length = strlen(str) + 1;
    char *new_string = (char *)allocate(arena, length);
    memcpy(new_string, str, length);
    new_string[length - 1] = '\0';
    return new_string;
}