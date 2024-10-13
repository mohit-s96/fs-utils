#!/bin/bash

# Compile with debug symbols (-g)
# Disable library validation (-fno-sanitize=safe-stack)
# Include all warning messages (-Wall)
# Treat warnings as errors (-Werror)

clang -g -fno-sanitize=safe-stack -Wall -Werror -o main.out main.c cli.c utils.c ls.c arena.c colors.c