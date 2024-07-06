#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../cli.h"

static void ls_default_success(void **state)
{
    char **argv = {(char **)"fsc"};
    int argc = 1;
    Cli_args *args = parse_cli(argc, argv);
    assert_string_equal(".", args->path);
    assert_int_equal(ls, args->command);
}

static void ls_path_success(void **state)
{
    char *argv[] = {"fsc", "/User/Downloads"};
    int argc = 2;
    Cli_args *args = parse_cli(argc, argv);
    assert_string_equal("/User/Downloads", args->path);
    assert_int_equal(ls, args->command);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(ls_default_success),
        cmocka_unit_test(ls_path_success),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}