CC = clang
CFLAGS = -Wall -Werror
LDFLAGS = -fno-sanitize=safe-stack
SOURCES = main.c cli.c utils.c ls.c arena.c colors.c find.c pattern.c new.c
TEST_SOURCES = tests/test.c cli.c arena.c utils.c
EXECUTABLE = main.out
TEST_EXECUTABLE = tests/test.out
INSTALL_NAME = fs
INSTALL_PATH = /usr/local/bin

.PHONY: all clean test build dev release install

all: dev

dev: CFLAGS += -g
dev: $(EXECUTABLE)

release: CFLAGS += -O2
release: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

test: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): $(TEST_SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ -lcmocka

clean:
	rm -f $(EXECUTABLE) $(TEST_EXECUTABLE)

build: dev

# Alias for the original build.sh
build-sh:
	./build.sh

# Alias for the original test.sh
test-sh:
	./test.sh

install: release
	install -d $(DESTDIR)$(INSTALL_PATH)
	install -m 755 $(EXECUTABLE) $(DESTDIR)$(INSTALL_PATH)/$(INSTALL_NAME)