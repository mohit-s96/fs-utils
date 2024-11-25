#include "colors.h"

void print_red(int is_stdout)
{
    if (is_stdout)
        printf("\033[31m");
}

void print_green(int is_stdout)
{
    if (is_stdout)
        printf("\033[32m");
}

void print_yellow(int is_stdout)
{
    if (is_stdout)
        printf("\033[33m");
}

void print_blue(int is_stdout)
{
    if (is_stdout)
        printf("\033[34m");
}

void print_magenta(int is_stdout)
{
    if (is_stdout)
        printf("\033[35m");
}

void print_cyan(int is_stdout)
{
    if (is_stdout)
        printf("\033[36m");
}

void print_white(int is_stdout)
{
    if (is_stdout)
        printf("\033[37m");
}

void print_bold(int is_stdout)
{
    if (is_stdout)
        printf("\033[1m");
}

void print_underlined(int is_stdout)
{
    if (is_stdout)
        printf("\033[4m");
}

void print_invisible(int is_stdout)
{
    if (is_stdout)
        printf("\033[08m");
}

void print_reset(int is_stdout)
{
    if (is_stdout)
        printf("\033[0m");
}
