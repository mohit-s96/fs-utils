#pragma once

#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "arena.h"

typedef struct
{
    char *parent;
    char *child;
} Tuple;

void quick_sort(void *arr, size_t size, int low, int high, int (*cmp)(const void *, const void *));
bool is_symlink(mode_t mode);
long long get_dir_size(const char *path, int max_depth);
long long get_dir_size_threaded(char *path, int max_depth, Arena *arena);
char *get_user_name_from_uid(uid_t uid);
char *get_group_name_from_gid(gid_t gid);
char *join_paths(const char *path1, const char *path2, Arena *arena);
char *get_user_permissions(mode_t mode, Arena *arena);
void format_size(unsigned long long bytes, char *output, size_t output_size);
int safe_parse_cli_int(char *num);
bool check_if_parent_dir(char *dir_name);
char *duplicate_string(char *str, Arena *arena);
Tuple *parent_path_from_child(char *path, size_t length, Tuple *t, Arena *arena);
bool str_ends_with_char(char *str, int length, char ch);
char *parse_special_dir(char *path, int length);