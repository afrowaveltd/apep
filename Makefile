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

# rm -rf equivalent for directories
RMDIR_RF = if exist bin rmdir /S /Q bin

# Clean object files + library (ignore errors)
CLEAN_OBJ = del /Q src\*.o 2>NUL || exit 0
CLEAN_LIB = del /Q libapep.a 2>NUL || exit 0

# Windows libs
LDFLAGS := -lkernel32
else
EXE :=
MKDIR_BIN = mkdir -p bin
RMDIR_RF = rm -rf bin
CLEAN_OBJ = rm -f $(OBJ)
CLEAN_LIB = rm -f $(LIB)
endif

# Toolchain defaults
CC ?= gcc
ifeq ($(CC),cc)
CC := gcc
endif

AR ?= ar

# Use gnu11 to get POSIX-ish goodies where available (fileno, isatty, etc.)
CFLAGS ?= -std=gnu11 -O2 -Wall -Wextra -Wpedantic -Iinclude
LIB = libapep.a

# ---- Install settings ----
PREFIX   ?= /usr/local
DESTDIR  ?=
INCDIR   ?= $(PREFIX)/include
LIBDIR   ?= $(PREFIX)/lib

# For POSIX: default install tools
INSTALL      ?= install
INSTALL_DIR  ?= $(INSTALL) -d
INSTALL_DATA ?= $(INSTALL) -m 644

# Windows: use copy commands
ifeq ($(OS),Windows_NT)
PREFIX   := C:/Program Files/apep
INCDIR   := $(PREFIX)/include
LIBDIR   := $(PREFIX)/lib
INSTALL_DIR  := if not exist
INSTALL_DATA := copy
endif



SRC = \
    src/apep_caps.c \
    src/apep_color.c \
    src/apep_text.c \
    src/apep_hex.c \
    src/apep_util.c \
    src/apep_helpers.c \
    src/apep_i18n.c \
    src/apep_json.c \
    src/apep_filter.c \
    src/apep_buffer.c \
    src/apep_scheme.c \
    src/apep_stack.c \
    src/apep_suggest.c \
    src/apep_multispan.c \
    src/apep_perf.c \
    src/apep_progress.c \
    src/apep_assert.c

OBJ = $(SRC:.c=.o)

DEMO_TEXT        = bin/apep_text_demo$(EXE)
DEMO_HEX         = bin/apep_hex_demo$(EXE)
DEMO_LOG         = bin/apep_log_demo$(EXE)
DEMO_SHOW        = bin/apep_show_demo$(EXE)
DEMO_HELPERS     = bin/apep_helpers_demo$(EXE)
DEMO_LOGGER      = bin/apep_logger_demo$(EXE)
DEMO_I18N        = bin/apep_i18n_demo$(EXE)
DEMO_I18N_FULL   = bin/apep_i18n_comprehensive_demo$(EXE)
DEMO_NEW_FEATURES= bin/apep_new_features_demo$(EXE)

all: $(LIB) examples

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

# Ensure bin exists
bin:
	$(MKDIR_BIN)

src/%.o: src/%.c | bin
	$(CC) $(CFLAGS) -c $< -o $@

examples: $(LIB) | bin
	$(CC) $(CFLAGS) -o $(DEMO_TEXT)         examples/text_error_demo.c            $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_HEX)          examples/hex_error_demo.c             $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_LOG)          examples/log_demo.c                   $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_SHOW)         examples/show_demo.c                  $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_HELPERS)      examples/helpers_demo.c               $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_LOGGER)       examples/logger_wrapper.c             $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_I18N)         examples/i18n_demo.c                  $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_I18N_FULL)    examples/i18n_comprehensive_demo.c    $(LIB) $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(DEMO_NEW_FEATURES) examples/new_features_demo.c          $(LIB) $(LDFLAGS)

clean:
	$(CLEAN_OBJ)
	$(CLEAN_LIB)
	$(RMDIR_RF)

ifeq ($(OS),Windows_NT)
# Windows install
install: $(LIB)
	@echo Installing to: $(PREFIX)
	@if not exist "$(INCDIR)\apep" mkdir "$(INCDIR)\apep"
	@copy /Y include\apep\apep.h "$(INCDIR)\apep\apep.h"
	@copy /Y include\apep\apep_helpers.h "$(INCDIR)\apep\apep_helpers.h"
	@copy /Y include\apep\apep_i18n.h "$(INCDIR)\apep\apep_i18n.h"
	@if not exist "$(LIBDIR)" mkdir "$(LIBDIR)"
	@copy /Y $(LIB) "$(LIBDIR)\$(LIB)"
	@echo Done.

uninstall:
	@echo Uninstalling from: $(PREFIX)
	@if exist "$(LIBDIR)\$(LIB)" del /Q "$(LIBDIR)\$(LIB)"
	@if exist "$(INCDIR)\apep\apep.h" del /Q "$(INCDIR)\apep\apep.h"
	@if exist "$(INCDIR)\apep\apep_helpers.h" del /Q "$(INCDIR)\apep\apep_helpers.h"
	@if exist "$(INCDIR)\apep\apep_i18n.h" del /Q "$(INCDIR)\apep\apep_i18n.h"
	@if exist "$(INCDIR)\apep" rmdir /Q "$(INCDIR)\apep" 2>NUL
	@echo Done.
else
# POSIX install
install: $(LIB)
	@echo Installing to: $(DESTDIR)$(PREFIX)
	$(INSTALL_DIR)  "$(DESTDIR)$(INCDIR)/apep"
	$(INSTALL_DATA) include/apep/apep.h         "$(DESTDIR)$(INCDIR)/apep/apep.h"
	$(INSTALL_DATA) include/apep/apep_helpers.h "$(DESTDIR)$(INCDIR)/apep/apep_helpers.h"
	$(INSTALL_DATA) include/apep/apep_i18n.h    "$(DESTDIR)$(INCDIR)/apep/apep_i18n.h"
	$(INSTALL_DIR)  "$(DESTDIR)$(LIBDIR)"
	$(INSTALL_DATA) $(LIB) "$(DESTDIR)$(LIBDIR)/$(LIB)"
	@echo Done.

uninstall:
	@echo Uninstalling from: $(DESTDIR)$(PREFIX)
	-@rm -f "$(DESTDIR)$(LIBDIR)/$(LIB)"
	-@rm -f "$(DESTDIR)$(INCDIR)/apep/apep.h"
	-@rm -f "$(DESTDIR)$(INCDIR)/apep/apep_helpers.h"
	-@rm -f "$(DESTDIR)$(INCDIR)/apep/apep_i18n.h"
	-@rmdir "$(DESTDIR)$(INCDIR)/apep" 2>/dev/null || true
	@echo Done.
endif


.PHONY: all clean examples bin install uninstall
