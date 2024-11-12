#pragma once

#include "cli.h"
#include "arena.h"

int command_ls(Cli_args *args, Arena *arena);
int command_find(Cli_args *args, Arena *arena);
int command_new(Cli_args *args, Arena *arena);
int command_size(Cli_args *args, Arena *arena);
