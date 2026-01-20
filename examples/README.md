# APEP Examples Guide

This directory contains demonstration programs showcasing APEP features.

## Overview

| Program | Purpose | Key Features |
|---------|---------|--------------|
| **show_demo** | Full feature showcase | All APEP capabilities in one demo |
| **text_error_demo** | Basic text diagnostics | Syntax errors, multi-line spans |
| **hex_error_demo** | Binary/hex diagnostics | Byte-level errors, hex dumps |
| **helpers_demo** | Helper macros | APEP_ERROR, APEP_ASSERT |
| **log_demo** | Logging integration | Building loggers with APEP |
| **exception_demo** | Exception handling | .NET/Java-style exceptions with stack traces |
| **i18n_demo** | Basic i18n | Language switching |
| **i18n_comprehensive_demo** | Advanced i18n | Full translation system |
| **new_features_demo** | Latest additions | Newest APEP features |

## Learning Path

### 1. Beginners → show_demo
Start here for complete overview.

```sh
./bin/show_demo              # Full colors
./bin/show_demo --plain      # ASCII only
```

### 2. Text Errors → text_error_demo
Learn basic diagnostics.

```sh
./bin/text_error_demo
```

### 3. Binary Errors → hex_error_demo
Hexdump-style diagnostics.

```sh
./bin/hex_error_demo
```

### 4. Helper Macros → helpers_demo
Production patterns with helpers.

```sh
./bin/helpers_demo
```

### 5. Logging → log_demo
Build logging systems.

### 6. Exception Handling → exception_demo
.NET/Java-style exception handling with stack traces and chaining.

```sh
./bin/exception_demo
```

### 7. Internationalization → i18n_demo
Language support basics.

```sh
./bin/i18n_demo --lang cs
./bin/i18n_demo --lang en
```

### 8. Advanced i18n → i18n_comprehensive_demo
Complete translation system.

```sh
./bin/i18n_comprehensive_demo
```

### 9. Latest Features → new_features_demo
Cutting-edge APEP capabilities.

```sh
./bin/new_features_demo
```

## Building Examples

### With Makefile

```sh
make examples        # Build all
make show_demo       # Build specific
```

### With CMake

```sh
mkdir build && cd build
cmake ..
cmake --build . --target show_demo
```

### Individual Compilation

```sh
gcc -I../include examples/show_demo.c ../bin/libapep.a -o show_demo
```

## Detailed Descriptions

### show_demo.c
**Purpose**: Comprehensive feature showcase  
**Topics**: All APEP features  
**Output**: 500+ lines demonstrating every capability

**Features Shown**:
- Text diagnostics
- Hex diagnostics
- Multi-span errors
- Color schemes
- Helper macros
- Internationalization
- Suggestions
- Assertions
- Progress bars
- Performance counters

### text_error_demo.c
**Purpose**: Basic text error reporting  
**Topics**: Syntax errors, parsing errors

**Examples**:

```c
// Shows: Unexpected token ','
void show_syntax_error() { /* ... */ }

// Shows: Multi-line error spans
void show_multiline_error() { /* ... */ }
```

### hex_error_demo.c
**Purpose**: Binary data diagnostics  
**Topics**: Byte errors, hex dumps

**Examples**:

```c
// Shows: Invalid magic number in header
void show_binary_error() { /* ... */ }

// Shows: Checksum mismatch with hex dump
void show_checksum_error() { /* ... */ }
```

### helpers_demo.c
**Purpose**: Production helper macros  
**Topics**: APEP_ERROR, APEP_ASSERT, APEP_TRY

**Examples**:

```c
// Error with return
APEP_ERROR(ctx, "Failed to open %s", filename);
return -1;

// Assertion
APEP_ASSERT(ctx, ptr != NULL, "Null pointer");

// Try macro
APEP_TRY(ctx, fopen(file, "r"));
```

### log_demo.c
**Purpose**: Building loggers on APEP  
**Topics**: Custom log levels, multi-sink output

**Architecture**:
```
Logger
├── Console sink
├── File sink
└── APEP formatter
```

See [docs/LOGGER_INTEGRATION.md](../docs/LOGGER_INTEGRATION.md) for details.

### exception_demo.c
**Purpose**: Exception handling with stack traces and chaining  
**Topics**: .NET/Java-style exceptions, stack traces, exception chaining

**Features**:
- Exception types and messages
- Stack trace capture (platform-specific)
- Exception chaining (inner exceptions/causes)
- Source location tracking
- Error codes
- Helper macros: APEP_EXCEPTION, APEP_EXCEPTION_RETURN, APEP_EXCEPTION_WRAP

**Example**:

```c
// Simple exception with macro
APEP_EXCEPTION(&opt, "NullPointerException", 
              "Pointer was NULL at offset %d", 42);

// Exception chaining
apep_exception_t *inner = apep_exception_create(
    "IOException", "Connection timeout");
apep_exception_set_source(inner, __FILE__, __LINE__);

apep_exception_t *outer = apep_exception_create(
    "DatabaseException", "Failed to connect");
apep_exception_set_inner(outer, inner);
apep_exception_capture_stack(outer);

apep_exception_print_chain(&opt, outer, 0);
apep_exception_destroy(outer);
```

### i18n_demo.c
**Purpose**: Basic internationalization  
**Topics**: Language loading, string translation

**Usage**:

```sh
./bin/i18n_demo --lang cs    # Czech
./bin/i18n_demo --lang en    # English
```

### i18n_comprehensive_demo.c
**Purpose**: Complete i18n system  
**Topics**: Locale fallback, plurals, custom translations

**Features**:
- Auto-detection
- Fallback chains
- Custom strings
- Runtime switching

See [docs/I18N.md](../docs/I18N.md) for details.

### new_features_demo.c
**Purpose**: Latest APEP additions  
**Topics**: Newest features

Check changelog for current features.

## Common Tasks

### Add New Example

1. Create `myexample.c` in `examples/`
2. Add to `Makefile`:

```make
   myexample: $(EXAMPLES_DIR)/myexample.c $(LIB)
       $(CC) -I$(INCLUDE_DIR) $< $(LIB) -o $(BIN_DIR)/$@
```

3. Add to `CMakeLists.txt`:

```cmake
   add_executable(myexample examples/myexample.c)
   target_link_libraries(myexample apep)
```

### Debug Example

```sh
# Linux/macOS
gdb ./bin/show_demo
valgrind --leak-check=full ./bin/show_demo

# Windows (MinGW)
gdb ./bin/show_demo.exe
```

## Troubleshooting

### Build Errors

**Error**: `apep.h: No such file or directory`  
**Fix**: `gcc -I../include ...`

**Error**: `undefined reference to apep_*`  
**Fix**: Link with library: `gcc ... ../bin/libapep.a`

### Runtime Errors

**Error**: Garbled colors  
**Fix**: Run with `--plain` or check terminal capabilities

**Error**: Missing translations  
**Fix**: Ensure `locales/*.loc` files exist

## Integration Examples

See [docs/IMPLEMENTATION_EXAMPLES.md](../docs/IMPLEMENTATION_EXAMPLES.md) for real-world usage patterns.

## Contributing

To add examples:
1. Follow existing structure
2. Add comments explaining logic
3. Keep under 300 lines
4. Test on Windows/Linux/macOS

See [CONTRIBUTING.md](../CONTRIBUTING.md) for details.