rm -f test
clang -o test tests/test.c cli.c -lcmocka
./test