#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../cli.h"
#include "../arena.h"

Arena arena;

static void ls_implicit_default_success(void **state)
{
    char **argv = {(char **)"fsc"};
    int argc = 1;
    Cli_args *args = parse_cli(argc, argv, &arena);
    assert_int_equal(LS, args->command);
    assert_string_equal(".", args->path);
}

static void ls_implicit_path_success(void **state)
{
    char *argv[] = {"fsc", "/User/Downloads"};
    int argc = 2;
    Cli_args *args = parse_cli(argc, argv, &arena);
    assert_int_equal(LS, args->command);
    assert_string_equal("/User/Downloads", args->path);
}

static void ls_implicit_flags_no_path_success(void **state)
{
    char *argv[] = {"fsc", "-s"};
    int argc = 2;
    Cli_args *args = parse_cli(argc, argv, &arena);
    assert_int_equal(LS, args->command);
    assert_string_equal(".", args->path);
    assert_true(args->sort_by_size);
}

static void ls_explicit_flags_path_success(void **state)
{
    char *argv[] = {"fsc", "ls", "/User/Downloads", "-a", "-s"};
    int argc = 5;
    Cli_args *args = parse_cli(argc, argv, &arena);
    assert_int_equal(LS, args->command);
    assert_string_equal("/User/Downloads", args->path);
    assert_true(args->sort_by_size);
    assert_true(args->sort_by_name);
}

static void find_path_and_file_success(void **state)
{
    char *argv[] = {"fsc", "f", "/User/Downloads", "--file", "hello.c"};
    int argc = 5;
    Cli_args *args = parse_cli(argc, argv, &arena);
    assert_int_equal(F, args->command);
    assert_string_equal("/User/Downloads", args->path);
    assert_string_equal("hello.c", args->search_pattern);
}

static void find_path_and_file_no_recurse_success(void **state)
{
    char *argv[] = {"fsc", "f", "/User/Downloads", "--file", "hello.c", "-nr"};
    int argc = 6;
    Cli_args *args = parse_cli(argc, argv, &arena);
    assert_int_equal(F, args->command);
    assert_string_equal("/User/Downloads", args->path);
    assert_string_equal("hello.c", args->search_pattern);
    assert_true(args->no_recurse);
}

static void copy_success(void **state)
{
    char *argv[] = {"fsc", "cp", "/User/Downloads/hello.c", "."};
    int argc = 4;
    Cli_args *args = parse_cli(argc, argv, &arena);
    assert_int_equal(CP, args->command);
    assert_string_equal("/User/Downloads/hello.c", args->source);
    assert_string_equal(".", args->destination);
}

static void move_success(void **state)
{
    char *argv[] = {"fsc", "mv", "/User/Downloads/hello.c", "."};
    int argc = 4;
    Cli_args *args = parse_cli(argc, argv, &arena);
    assert_int_equal(MV, args->command);
    assert_string_equal("/User/Downloads/hello.c", args->source);
    assert_string_equal(".", args->destination);
}

static void copy_empty_destination_success(void **state)
{
    char *argv[] = {"fsc", "cp", "/User/Downloads/hello.c"};
    int argc = 3;
    Cli_args *args = parse_cli(argc, argv, &arena);
    assert_int_equal(CP, args->command);
    assert_string_equal("/User/Downloads/hello.c", args->source);
    assert_true(NULL == args->destination);
}

static void new_file_success(void **state)
{
    char *argv[] = {"fsc", "new", "hello.c"};
    int argc = 3;
    Cli_args *args = parse_cli(argc, argv, &arena);
    assert_int_equal(NEW, args->command);
    assert_string_equal("hello.c", args->new_file_name);
    assert_true(NULL == args->new_dir_name);
}

static void new_dir_success(void **state)
{
    char *argv[] = {"fsc", "new", "-d", "hello"};
    int argc = 4;
    Cli_args *args = parse_cli(argc, argv, &arena);
    assert_int_equal(NEW, args->command);
    assert_string_equal("hello", args->new_dir_name);
    assert_true(NULL == args->new_file_name);
}

static void stat_success(void **state)
{
    char *argv[] = {"fsc", "size", "."};
    int argc = 3;
    Cli_args *args = parse_cli(argc, argv, &arena);
    assert_int_equal(SIZE, args->command);
    assert_string_equal(".", args->path);
}

int main(void)
{
    init_arena(&arena, 1024);
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(ls_implicit_default_success),
        cmocka_unit_test(ls_implicit_path_success),
        cmocka_unit_test(ls_implicit_flags_no_path_success),
        cmocka_unit_test(ls_explicit_flags_path_success),
        cmocka_unit_test(find_path_and_file_success),
        cmocka_unit_test(find_path_and_file_no_recurse_success),
        cmocka_unit_test(copy_success),
        cmocka_unit_test(move_success),
        cmocka_unit_test(copy_empty_destination_success),
        cmocka_unit_test(new_file_success),
        cmocka_unit_test(new_dir_success),
        cmocka_unit_test(stat_success),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}