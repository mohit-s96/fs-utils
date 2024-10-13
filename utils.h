#pragma once

#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "arena.h"

void quick_sort(void *arr, size_t size, int low, int high, int (*cmp)(const void *, const void *));
bool is_symlink(mode_t mode);
long long get_dir_size(const char *path, int max_depth);
char *get_user_name_from_uid(uid_t uid);
char *get_group_name_from_gid(gid_t gid);
char *join_paths(const char *path1, const char *path2, Arena *arena);
char *get_user_permissions(mode_t mode, Arena *arena);
void format_size(unsigned long long bytes, char *output, size_t output_size);
int safe_parse_cli_int(char *num);