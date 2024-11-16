# Detect compiler: prefer clang, fallback to gcc
CC := $(shell command -v clang >/dev/null 2>&1 && echo clang || echo gcc)

# Compiler-specific flags
ifeq ($(CC),clang)
    LDFLAGS = -fno-sanitize=safe-stack
else
    LDFLAGS =
endif

CFLAGS = -Wall -Werror
SOURCES = main.c cli.c utils.c ls.c arena.c colors.c find.c pattern.c new.c size.c threads.c copy.c
TEST_SOURCES = tests/test.c cli.c arena.c utils.c threads.c
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

install: release
	install -d $(DESTDIR)$(INSTALL_PATH)
	install -m 755 $(EXECUTABLE) $(DESTDIR)$(INSTALL_PATH)/$(INSTALL_NAME)
