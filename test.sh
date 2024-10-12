rm -f tests/test.out
clang -o tests/test.out tests/test.c cli.c arena.c -lcmocka
./tests/test.out