# ----------------------------
# APEP Makefile (portable)
# - works on Linux/macOS (GNU make)
# - works on Windows (mingw32-make)
# ----------------------------

# Detect Windows (cmd) vs POSIX shell
ifeq ($(OS),Windows_NT)
	SHELL := cmd
	.SHELLFLAGS := /C
	EXE := .exe
	# mkdir -p equivalent
	MKDIR_BIN = if not exist bin mkdir bin
	# rm -f equivalent for individual files
	RM_F = del /Q
	# Clean object files
	CLEAN_OBJ = del /Q src\apep_caps.o src\apep_color.o src\apep_text.o src\apep_hex.o src\apep_util.o src\apep_helpers.o 2>NUL || exit 0
	# Clean library
	CLEAN_LIB = del /Q libapep.a 2>NUL || exit 0
	# rm -rf equivalent for directories
	RMDIR_RF = if exist bin rmdir /S /Q bin
else
	EXE :=
	MKDIR_BIN = mkdir -p bin
	RM_F = rm -f
	CLEAN_OBJ = rm -f $(OBJ)
	CLEAN_LIB = rm -f $(LIB)
	RMDIR_RF = rm -rf bin
endif

# Toolchain defaults
# Note: some environments set CC=cc by default.
CC ?= gcc
ifeq ($(CC),cc)
	CC := gcc
endif

AR ?= ar
# Use gnu11 instead of c11 to get POSIX extensions like fileno, isatty
CFLAGS ?= -std=gnu11 -O2 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS ?=
LIB = libapep.a

SRC = \
	src/apep_caps.c \
	src/apep_color.c \
	src/apep_text.c \
	src/apep_hex.c \
	src/apep_util.c \
	src/apep_helpers.c

OBJ = $(SRC:.c=.o)

DEMO_TEXT = bin/apep_text_demo$(EXE)
DEMO_HEX = bin/apep_hex_demo$(EXE)
DEMO_LOG = bin/apep_log_demo$(EXE)
DEMO_SHOW = bin/apep_show_demo$(EXE)
DEMO_HELPERS = bin/apep_helpers_demo$(EXE)
DEMO_LOGGER = bin/apep_logger_demo$(EXE)

all: $(LIB) examples

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

# Ensure bin exists
bin:
	$(MKDIR_BIN)

src/%.o: src/%.c | bin
	$(CC) $(CFLAGS) -c $< -o $@

examples: $(LIB) | bin
	$(CC) $(CFLAGS) -o $(DEMO_TEXT) examples/text_error_demo.c $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_HEX) examples/hex_error_demo.c $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_LOG) examples/log_demo.c $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_SHOW) examples/show_demo.c $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_HELPERS) examples/helpers_demo.c $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_LOGGER) examples/logger_wrapper.c $(LIB) $(LDFLAGS)

clean:
	$(CLEAN_OBJ)
	$(CLEAN_LIB)
	$(RMDIR_RF)

.PHONY: all clean examples bin