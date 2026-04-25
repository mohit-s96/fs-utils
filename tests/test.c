#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../arena.h"
#include "../cli.h"

typedef struct
{
    int exit_code;
    char output[8192];
} CommandResult;

static Arena arena;

static int setup(void **state)
{
    (void)state;
    init_arena(&arena, 64 * 1024);
    return 0;
}

static int teardown(void **state)
{
    (void)state;
    destroy(&arena);
    return 0;
}

static char *duplicate_string(const char *input)
{
    size_t length = strlen(input) + 1;
    char *copy = malloc(length);
    assert_non_null(copy);
    memcpy(copy, input, length);
    return copy;
}

static char *make_temp_dir(void)
{
    char template[] = "/tmp/fs-c-tests-XXXXXX";
    char *path = duplicate_string(template);
    assert_non_null(mkdtemp(path));
    return path;
}

static char *join_path(const char *left, const char *right)
{
    size_t left_length = strlen(left);
    size_t right_length = strlen(right);
    size_t needs_separator = left_length > 0 && left[left_length - 1] != '/';
    size_t total = left_length + right_length + needs_separator + 1;
    char *path = malloc(total);
    assert_non_null(path);

    snprintf(path, total, needs_separator ? "%s/%s" : "%s%s", left, right);
    return path;
}

static void write_text_file(const char *path, const char *contents)
{
    FILE *file = fopen(path, "wb");
    assert_non_null(file);
    assert_int_equal(strlen(contents), fwrite(contents, 1, strlen(contents), file));
    fclose(file);
}

static char *read_text_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    assert_non_null(file);

    assert_int_equal(fseek(file, 0, SEEK_END), 0);
    long length = ftell(file);
    assert_true(length >= 0);
    assert_int_equal(fseek(file, 0, SEEK_SET), 0);

    char *buffer = malloc((size_t)length + 1);
    assert_non_null(buffer);
    assert_int_equal((size_t)length, fread(buffer, 1, (size_t)length, file));
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

static void assert_path_exists(const char *path)
{
    struct stat st;
    assert_int_equal(0, lstat(path, &st));
}

static void assert_path_missing(const char *path)
{
    struct stat st;
    assert_int_not_equal(0, lstat(path, &st));
}

static void remove_path_recursive(const char *path)
{
    struct stat st;
    if (lstat(path, &st) != 0)
    {
        return;
    }

    if ((st.st_mode & S_IFMT) != S_IFDIR)
    {
        assert_int_equal(0, unlink(path));
        return;
    }

    DIR *dir = opendir(path);
    assert_non_null(dir);

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char *child = join_path(path, entry->d_name);
        remove_path_recursive(child);
        free(child);
    }

    closedir(dir);
    assert_int_equal(0, rmdir(path));
}

static CommandResult run_cli(const char *format, ...)
{
    CommandResult result = {0};
    char command[PATH_MAX * 3];
    va_list args;

    va_start(args, format);
    vsnprintf(command, sizeof(command), format, args);
    va_end(args);

    FILE *pipe = popen(command, "r");
    assert_non_null(pipe);

    size_t used = 0;
    while (used < sizeof(result.output) - 1)
    {
        size_t read = fread(result.output + used, 1, sizeof(result.output) - 1 - used, pipe);
        used += read;
        if (read == 0)
        {
            break;
        }
    }
    result.output[used] = '\0';

    int status = pclose(pipe);
    assert_true(status != -1);
    if (WIFEXITED(status))
    {
        result.exit_code = WEXITSTATUS(status);
    }
    else
    {
        result.exit_code = -1;
    }

    return result;
}

static void ls_implicit_default_success(void **state)
{
    (void)state;
    char *argv[] = {"fsc"};
    Cli_args *args = parse_cli(1, argv, &arena);
    assert_int_equal(LS, args->command);
    assert_string_equal(".", args->path);
}

static void ls_explicit_flags_success(void **state)
{
    (void)state;
    char *argv[] = {"fsc", "ls", "/User/Downloads", "-a", "-s"};
    Cli_args *args = parse_cli(5, argv, &arena);
    assert_int_equal(LS, args->command);
    assert_true(args->sort_by_size);
    assert_true(args->sort_by_name);
}

static void ls_compact_sets_depth_zero(void **state)
{
    (void)state;
    char *argv[] = {"fsc", "ls", "-c"};
    Cli_args *args = parse_cli(3, argv, &arena);
    assert_int_equal(LS, args->command);
    assert_true(args->compact);
    assert_int_equal(0, args->depth);
}

static void find_defaults_to_current_dir_when_pattern_is_first(void **state)
{
    (void)state;
    char *argv[] = {"fsc", "f", "-p", "*.c", "--depth", "3", "-nr"};
    Cli_args *args = parse_cli(7, argv, &arena);
    assert_int_equal(F, args->command);
    assert_string_equal("*.c", args->search_pattern);
    assert_int_equal(3, args->depth);
    assert_true(args->no_recurse);
}

static void copy_parser_success(void **state)
{
    (void)state;
    char *argv[] = {"fsc", "cp", "/User/Downloads/hello.c", "."};
    Cli_args *args = parse_cli(4, argv, &arena);
    assert_int_equal(CP, args->command);
    assert_string_equal("/User/Downloads/hello.c", args->source);
    assert_string_equal(".", args->destination);
}

static void new_dir_success(void **state)
{
    (void)state;
    char *argv[] = {"fsc", "new", "-d", "hello"};
    Cli_args *args = parse_cli(4, argv, &arena);
    assert_int_equal(NEW, args->command);
    assert_string_equal("hello", args->new_dir_name[0]);
    assert_true(args->new_file_name[0] == NULL);
}

static void size_implicit_default_success(void **state)
{
    (void)state;
    char *argv[] = {"fsc", "size"};
    Cli_args *args = parse_cli(2, argv, &arena);
    assert_int_equal(SIZE, args->command);
    assert_string_equal(".", args->path);
}

static void help_command_lists_supported_commands(void **state)
{
    (void)state;
    CommandResult result = run_cli("./main.out help 2>&1");
    assert_int_equal(EXIT_SUCCESS, result.exit_code);
    assert_non_null(strstr(result.output, "Usage:"));
    assert_non_null(strstr(result.output, "ls"));
    assert_non_null(strstr(result.output, "f"));
    assert_non_null(strstr(result.output, "cp"));
    assert_non_null(strstr(result.output, "mv"));
    assert_non_null(strstr(result.output, "new"));
    assert_non_null(strstr(result.output, "size"));
}

static void new_command_creates_files_and_directories(void **state)
{
    (void)state;
    char *temp_dir = make_temp_dir();
    char *file_one = join_path(temp_dir, "one.txt");
    char *file_two = join_path(temp_dir, "two.txt");
    char *dir_one = join_path(temp_dir, "alpha");
    char *dir_two = join_path(temp_dir, "beta");

    CommandResult files = run_cli("./main.out new %s %s 2>&1", file_one, file_two);
    assert_int_equal(EXIT_SUCCESS, files.exit_code);
    assert_path_exists(file_one);
    assert_path_exists(file_two);

    CommandResult dirs = run_cli("./main.out new -d %s %s 2>&1", dir_one, dir_two);
    assert_int_equal(EXIT_SUCCESS, dirs.exit_code);
    assert_path_exists(dir_one);
    assert_path_exists(dir_two);

    remove_path_recursive(temp_dir);
    free(dir_two);
    free(dir_one);
    free(file_two);
    free(file_one);
    free(temp_dir);
}

static void size_command_reports_file_size(void **state)
{
    (void)state;
    char *temp_dir = make_temp_dir();
    char *file_path = join_path(temp_dir, "size.txt");
    write_text_file(file_path, "data");

    CommandResult result = run_cli("printf '%s\\n' | ./main.out size 2>&1", file_path);
    assert_int_equal(EXIT_SUCCESS, result.exit_code);
    assert_string_equal("4.0B\n", result.output);

    remove_path_recursive(temp_dir);
    free(file_path);
    free(temp_dir);
}

static void find_command_honors_no_recurse(void **state)
{
    (void)state;
    char *temp_dir = make_temp_dir();
    char *top_match = join_path(temp_dir, "match.txt");
    char *nested_dir = join_path(temp_dir, "nested");
    char *nested_match = NULL;

    write_text_file(top_match, "top");
    assert_int_equal(0, mkdir(nested_dir, 0700));
    nested_match = join_path(nested_dir, "match.txt");
    write_text_file(nested_match, "nested");

    CommandResult result = run_cli("printf '%s\\n' | ./main.out f --file match.txt -nr 2>&1", temp_dir);
    assert_int_equal(EXIT_SUCCESS, result.exit_code);
    assert_non_null(strstr(result.output, top_match));
    assert_null(strstr(result.output, nested_match));

    remove_path_recursive(temp_dir);
    free(nested_match);
    free(nested_dir);
    free(top_match);
    free(temp_dir);
}

static void copy_command_copies_file_into_directory(void **state)
{
    (void)state;
    char *temp_dir = make_temp_dir();
    char *source = join_path(temp_dir, "source.txt");
    char *destination_dir = join_path(temp_dir, "dest");
    char *copied_file = NULL;
    char *contents = NULL;

    write_text_file(source, "hello copy");
    assert_int_equal(0, mkdir(destination_dir, 0700));

    CommandResult result = run_cli("./main.out cp %s %s 2>&1", source, destination_dir);
    assert_int_equal(EXIT_SUCCESS, result.exit_code);

    copied_file = join_path(destination_dir, "source.txt");
    assert_path_exists(copied_file);
    contents = read_text_file(copied_file);
    assert_string_equal("hello copy", contents);

    free(contents);
    free(copied_file);
    remove_path_recursive(temp_dir);
    free(destination_dir);
    free(source);
    free(temp_dir);
}

static void move_command_moves_file_into_directory(void **state)
{
    (void)state;
    char *temp_dir = make_temp_dir();
    char *source = join_path(temp_dir, "move-me.txt");
    char *destination_dir = join_path(temp_dir, "dest");
    char *moved_file = NULL;
    char *contents = NULL;

    write_text_file(source, "hello move");
    assert_int_equal(0, mkdir(destination_dir, 0700));

    CommandResult result = run_cli("./main.out mv %s %s 2>&1", source, destination_dir);
    assert_int_equal(EXIT_SUCCESS, result.exit_code);

    moved_file = join_path(destination_dir, "move-me.txt");
    assert_path_exists(moved_file);
    assert_path_missing(source);
    contents = read_text_file(moved_file);
    assert_string_equal("hello move", contents);

    free(contents);
    free(moved_file);
    remove_path_recursive(temp_dir);
    free(destination_dir);
    free(source);
    free(temp_dir);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(ls_implicit_default_success, setup, teardown),
        cmocka_unit_test_setup_teardown(ls_explicit_flags_success, setup, teardown),
        cmocka_unit_test_setup_teardown(ls_compact_sets_depth_zero, setup, teardown),
        cmocka_unit_test_setup_teardown(find_defaults_to_current_dir_when_pattern_is_first, setup, teardown),
        cmocka_unit_test_setup_teardown(copy_parser_success, setup, teardown),
        cmocka_unit_test_setup_teardown(new_dir_success, setup, teardown),
        cmocka_unit_test_setup_teardown(size_implicit_default_success, setup, teardown),
        cmocka_unit_test_setup_teardown(help_command_lists_supported_commands, setup, teardown),
        cmocka_unit_test_setup_teardown(new_command_creates_files_and_directories, setup, teardown),
        cmocka_unit_test_setup_teardown(size_command_reports_file_size, setup, teardown),
        cmocka_unit_test_setup_teardown(find_command_honors_no_recurse, setup, teardown),
        cmocka_unit_test_setup_teardown(copy_command_copies_file_into_directory, setup, teardown),
        cmocka_unit_test_setup_teardown(move_command_moves_file_into_directory, setup, teardown),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
