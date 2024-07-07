rm -f tests/test.out
clang -o tests/test.out tests/test.c cli.c -lcmocka
./tests/test.out