#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include "threads.h"
#include "utils.h"

typedef struct
{
    unsigned long long size;
    pthread_mutex_t *mutex;
    Arena *arena;
} WorkerArgs;

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

bool is_symlink(mode_t mode)
{
    return (mode & S_IFMT) == S_IFLNK;
}

bool check_if_parent_dir(char *dir_name)
{
    return strcmp(dir_name, ".") == 0 || strcmp(dir_name, "..") == 0;
}

void *work_size(void *arg)
{
    WorkerArgs *worker_args = (WorkerArgs *)arg;
    struct stat st;
    char full_path[1024];
    char *path;
    while ((path = (char *)dequeue()) != NULL)
    {
        struct dirent *d;
        DIR *dir;
        dir = opendir(path);
        if (dir == NULL)
        {
            pthread_mutex_lock(get_queue_mutex());
            set_active_tasks(get_active_tasks() - 1); // Decrement active tasks if directory open fails
            pthread_mutex_unlock(get_queue_mutex());
            continue;
        }
        while ((d = readdir(dir)) != NULL)
        {
            bool is_dir = d->d_type == DT_DIR;
            char *dir_name = d->d_name;
            snprintf(full_path, sizeof(full_path), "%s/%s", path, dir_name);
            if (lstat(full_path, &st) == 0)
            {
                if (is_dir && !check_if_parent_dir(dir_name))
                {
                    char *new_path = join_paths(path, dir_name, worker_args->arena);
                    enqueue(new_path);
                }
                else
                {
                    pthread_mutex_lock(worker_args->mutex);
                    worker_args->size += st.st_blocks * 512;
                    pthread_mutex_unlock(worker_args->mutex);
                }
            }
        }
        closedir(dir);
        pthread_mutex_lock(get_queue_mutex());
        set_active_tasks(get_active_tasks() - 1);

        // If no tasks are left, signal all threads to exit
        if (get_active_tasks() == 0 && get_queue_size() == 0)
        {
            set_done(true);
            pthread_cond_broadcast(get_not_empty_condition()); // Notify all waiting threads
        }
        pthread_mutex_unlock(get_queue_mutex());
    }
    return NULL;
}

long long get_dir_size_threaded(char *path, int max_depth, Arena *arena)
{
    long number_of_processors = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t threads[number_of_processors];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    WorkerArgs args = {.mutex = &mutex, .size = 0, .arena = arena};

    enqueue(path);

    for (int i = 0; i < number_of_processors; i++)
    {
        if (pthread_create(&threads[i], NULL, work_size, &args) != 0)
        {
            fprintf(stderr, "Failed to create thread %d\n", i);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < number_of_processors; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(get_queue_mutex());
    pthread_cond_destroy(get_not_empty_condition());
    pthread_cond_destroy(get_not_full_condition());
    return args.size;
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

Tuple *parent_path_from_child(char *path, size_t length, Tuple *t, Arena *arena)
{
    t->parent = NULL;
    t->child = NULL;
    char *parent_path = (char *)allocate(arena, length + 1);
    char *child_path = (char *)allocate(arena, length + 1);
    parent_path[length] = '\0';
    if (length == 1 && path[0] == '/')
    {
        parent_path[0] = '/';
        parent_path[1] = '\0';
        t->parent = parent_path;
        return t;
    }
    int i = length;
    if (path[length - 1] == '/')
    {
        i--;
    }

    while (i-- > 0)
    {
        if (path[i] == '/')
        {
            break;
        }
    }

    if (i == 0)
    {
        parent_path[0] = '/';
        parent_path[1] = '\0';
        t->parent = parent_path;
        return t;
    }
    if (i < 0)
        return t;

    strncpy(parent_path, path, i);
    t->parent = parent_path;
    if (length - 1 - i - 1 > 0)
    {
        strncpy(child_path, path + i + 1, path[length - 1] == '/' ? length - i - 2 : length - i - 1);
        t->child = child_path;
    }
    return t;
}

bool str_ends_with_char(char *str, int length, char ch)
{
    return length > 0 && str[length - 1] == ch;
}

char *parse_special_dir(char *path, int length)
{
    if (length == 0)
        return NULL;
    if (length == 1 && str_ends_with_char(path, 1, '.'))
        return ".";
    if (length == 2 && str_ends_with_char(path, 2, '.') && str_ends_with_char(path, 1, '.'))
        return "..";

    if (str_ends_with_char(path, length, '/'))
        length--;

    if (str_ends_with_char(path, length, '.') && str_ends_with_char(path, length - 1, '/'))
        return ".";
    if (str_ends_with_char(path, length, '.') && str_ends_with_char(path, length - 1, '.') && str_ends_with_char(path, length - 2, '/'))
        return "..";

    return NULL;
}