# ==============================================================================
# Sneksphere - Professional C Makefile with Raylib & Clang LSP Support
# ==============================================================================

# Executable name
TARGET_NAME   ?= sneksphere

# Compiler & basic tools (clang by default, can override with 'make CC=gcc')
CC            := clang
RM            := rm -rf
MKDIR_P       := mkdir -p
PYTHON3       ?= python3

# Project directory structure
SRC_DIR       := src
INC_DIR       := include
OBJ_DIR       := objects
OUT_DIR       := out
SCRIPTS_DIR   := scripts

# Final executable binary
TARGET        := $(OUT_DIR)/$(TARGET_NAME)

# ------------------------------------------------------------------------------
# Automatic Source & Header Discovery (Zero Future Setup)
# ------------------------------------------------------------------------------
# Finds all .c files anywhere inside $(SRC_DIR) recursively
SRCS          := $(shell find $(SRC_DIR) -type f -name '*.c' 2>/dev/null)

# Map each src/path/to/file.c -> objects/path/to/file.o
OBJS          := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# Dependency files (.d) generated automatically during compilation
DEPS          := $(OBJS:.o=.d)

# Include flags for every subdirectory in $(INC_DIR)
INC_DIRS      := $(shell find $(INC_DIR) -type d 2>/dev/null)
INC_FLAGS     := $(addprefix -I,$(INC_DIRS))

# ------------------------------------------------------------------------------
# Raylib & Library Configuration
# ------------------------------------------------------------------------------
# Include ~/.local/lib/pkgconfig in PKG_CONFIG_PATH for user-installed libraries
export PKG_CONFIG_PATH := $(HOME)/.local/lib/pkgconfig:/usr/local/lib/pkgconfig:$(shell echo $$PKG_CONFIG_PATH)

# Prefer vendored raylib headers if present for direct definition indexing
ifneq ($(wildcard vendor/raylib/src/raylib.h),)
  RAYLIB_CFLAGS := -I$(CURDIR)/vendor/raylib/src
else
  # Query pkg-config for raylib flags
  RAYLIB_CFLAGS := $(shell pkg-config --cflags raylib 2>/dev/null)
endif

RAYLIB_LIBS   := $(shell pkg-config --libs raylib 2>/dev/null)

# Fallback paths if pkg-config cannot locate raylib
ifeq ($(strip $(RAYLIB_LIBS)),)
  ifeq ($(strip $(RAYLIB_CFLAGS)),)
    RAYLIB_CFLAGS := -I$(HOME)/.local/include
  endif
  RAYLIB_LIBS   := -L$(HOME)/.local/lib -lraylib
endif

# Linux system libraries required by Raylib (desktop OpenGL / X11 platform)
SYSTEM_LIBS   := -lm -lpthread -ldl -lrt -lX11

# ------------------------------------------------------------------------------
# Compilation and Linker Flags (Strict Warnings for C)
# ------------------------------------------------------------------------------
# Comprehensive warnings covering syntax, types, safety, and strict standards
CFLAGS_WARN   := -Wall \
                 -Wextra \
                 -Wpedantic \
                 -Wshadow \
                 -Wstrict-prototypes \
                 -Wmissing-prototypes \
                 -Wmissing-declarations \
                 -Wredundant-decls \
                 -Wnested-externs \
                 -Wpointer-arith \
                 -Wcast-align \
                 -Wwrite-strings \
                 -Wuninitialized \
                 -Wconversion \
                 -Wdouble-promotion \
                 -Wformat=2 \
                 -Wundef \
                 -Wimplicit-fallthrough \
                 -Wswitch-enum \
                 -Wvla \
                 -Wnull-dereference \
                 -Wstrict-overflow=2

CFLAGS_STD    := -std=c11

# Convert Raylib -I into -isystem so third-party headers don't spam warnings
RAYLIB_ISYSTEM := $(patsubst -I%,-isystem %,$(RAYLIB_CFLAGS))

# Preprocessor flags: includes, raylib headers, dependency tracking
CPPFLAGS      := $(INC_FLAGS) $(RAYLIB_ISYSTEM) -MMD -MP

# Build mode: 'debug' (default) or 'release'
# Usage: make MODE=release
MODE          ?= debug
ifeq ($(MODE),release)
  CFLAGS_OPT  := -O3 -DNDEBUG
else
  CFLAGS_OPT  := -g3 -O0 -DDEBUG
endif

CFLAGS        := $(CFLAGS_STD) $(CFLAGS_WARN) $(CFLAGS_OPT)
LDFLAGS       := $(RAYLIB_LIBS) $(SYSTEM_LIBS)

# ==============================================================================
# Targets
# ==============================================================================

.PHONY: all run clean fclean re compilecommands bear help

# Default target: builds executable and updates compile_commands.json for Neovim/Clang
all: $(TARGET) compile_commands.json

# Linking executable
$(TARGET): $(OBJS)
	@$(MKDIR_P) $(OUT_DIR)
	@echo "==> Linking: $@"
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "==> Build complete: $@"

# Compiling source files to objects (creates nested object subdirectories on the fly)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKDIR_P) $(dir $@)
	@echo "==> Compiling: $< -> $@"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Generate / update compile_commands.json for Clangd in Neovim
compile_commands.json: $(SRCS) $(SCRIPTS_DIR)/gen_compile_commands.py
	@echo "==> Updating compile_commands.json for Neovim / clangd..."
	@$(PYTHON3) $(SCRIPTS_DIR)/gen_compile_commands.py \
		--output "$@" \
		--cc "$(CC)" \
		--cflags "$(CFLAGS) $(CPPFLAGS)" \
		--src-dir "$(SRC_DIR)" \
		--obj-dir "$(OBJ_DIR)" \
		--sources "$(SRCS)"

# Alias for LSP database generation
compilecommands: compile_commands.json
compiledb: compile_commands.json

# Alternative generation using bear (if preferred)
bear: clean
	@echo "==> Generating compile_commands.json with bear..."
	@bear -- $(MAKE) $(TARGET)

# Run the compiled program
run: $(TARGET)
	@echo "==> Running $(TARGET)..."
	@./$(TARGET)

# Clean object files and output directory
clean:
	@echo "==> Cleaning build artifacts..."
	@$(RM) $(OBJ_DIR) $(OUT_DIR)

# Full clean including LSP database
fclean: clean
	@echo "==> Cleaning compile_commands.json..."
	@$(RM) compile_commands.json .cache

# Rebuild from scratch
re: clean all

# Show helpful usage instructions
help:
	@echo "Sneksphere Makefile Commands:"
	@echo "  make              - Build application and update compile_commands.json (debug mode)"
	@echo "  make MODE=release - Build application with -O3 optimizations"
	@echo "  make run          - Build and execute out/$(TARGET_NAME)"
	@echo "  make compilecommands - Generate compile_commands.json for clangd/neovim"
	@echo "  make bear         - Alternative generation of compile_commands.json using bear"
	@echo "  make clean        - Remove objects/ and out/"
	@echo "  make fclean       - Remove objects/, out/, and compile_commands.json"
	@echo "  make re           - Full rebuild"

# Include auto-generated dependency rules (skips if they do not exist yet)
-include $(DEPS)
