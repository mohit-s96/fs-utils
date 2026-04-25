# Detect compiler: prefer clang, fallback to gcc
CC := $(shell command -v clang >/dev/null 2>&1 && echo clang || echo gcc)
UNAME_S := $(shell uname -s)

# Compiler-specific flags
ifeq ($(CC),clang)
    LDFLAGS = -fno-sanitize=safe-stack
else
    LDFLAGS =
endif

CFLAGS = -Wall -Werror -pthread
SOURCES = main.c cli.c utils.c ls.c arena.c colors.c find.c pattern.c new.c size.c threads.c copy.c
TEST_SOURCES = tests/test.c cli.c arena.c utils.c threads.c
EXECUTABLE = main.out
TEST_EXECUTABLE = tests/test.out
INSTALL_NAME = fs
INSTALL_PATH = /usr/local/bin
CMOCKA_CFLAGS := $(shell pkg-config --cflags cmocka 2>/dev/null)
CMOCKA_LIBS := $(shell pkg-config --libs cmocka 2>/dev/null || echo -lcmocka)
TEST_LDFLAGS =

ifeq ($(UNAME_S),Darwin)
    CMOCKA_PREFIX := $(shell brew --prefix cmocka 2>/dev/null)
    ifneq ($(CMOCKA_PREFIX),)
        TEST_LDFLAGS += -Wl,-rpath,$(CMOCKA_PREFIX)/lib
    endif
    TEST_LDFLAGS += -Wl,-rpath,/opt/homebrew/lib -Wl,-rpath,/usr/local/lib
endif

.PHONY: all clean test build dev release install

all: dev

dev: CFLAGS += -g
dev: $(EXECUTABLE)

release: CFLAGS += -O2
release: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

test: $(EXECUTABLE) $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): $(TEST_SOURCES)
	$(CC) $(CFLAGS) $(CMOCKA_CFLAGS) $(TEST_LDFLAGS) -o $@ $^ $(CMOCKA_LIBS)

clean:
	rm -f $(EXECUTABLE) $(TEST_EXECUTABLE)

build: dev

install: release
	install -d $(DESTDIR)$(INSTALL_PATH)
	install -m 755 $(EXECUTABLE) $(DESTDIR)$(INSTALL_PATH)/$(INSTALL_NAME)
