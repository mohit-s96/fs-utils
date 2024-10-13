#include "colors.h"

void print_red()
{
    printf("\033[31m");
}

void print_green()
{
    printf("\033[32m");
}

void print_yellow()
{
    printf("\033[33m");
}

void print_blue()
{
    printf("\033[34m");
}

void print_magenta()
{
    printf("\033[35m");
}

void print_cyan()
{
    printf("\033[36m");
}

void print_white()
{
    printf("\033[37m");
}

void print_bold()
{
    printf("\033[1m");
}

void print_underlined()
{
    printf("\033[4m");
}

void print_invisible()
{
    printf("\033[08m");
}

void print_reset()
{
    printf("\033[0m");
}
