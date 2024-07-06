rm -f main
clang -o main main.c cli.c
./main $@