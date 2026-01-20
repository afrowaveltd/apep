# APEP vs Other Tools

This document compares APEP with similar libraries and tools in the C ecosystem.

---

## Comparison Matrix

| Feature | APEP | printf | Log.c | spdlog | ariadne/miette |
|---------|------|--------|-------|--------|---|
| **Language** | C | C | C | C++ | Rust |
| **Dependencies** | Zero | Standard C | Zero | Many | Built-in |
| **Text Diagnostics** | ✓ | ✗ | ~ | ~ | ✓ |
| **Binary Diagnostics** | ✓ | ✗ | ✗ | ✗ | ✗ |
| **Colors** | ✓ | ✗ | ✓ | ✓ | ✓ |
| **JSON Output** | ✓ | ✗ | ✗ | ✓ | ✗ |
| **Localization** | ✓ | ✗ | ✗ | ✗ | ✗ |
| **Thread-safe** | Partial | ✓ | ✓ | ✓ | ✓ |
| **License** | AHPL | - | MIT | BSD-3 | MIT |
| **Compile time** | <1s | <1s | <1s | ~5s | - |

---

## Detailed Comparisons

### APEP vs printf

**When to use printf:**
- Simple debug output
- Doesn't require formatting or context
- Compatibility with minimal libc implementations

**When to use APEP:**
- Error diagnostics with source context
- Structured, formatted output
- Hex dumps with annotations
- Multiple severity levels
- Localized messages

**Example:**

```c
// printf approach
printf("Error: bad token at position %d\n", pos);

// APEP approach
apep_error_t err;
apep_error_init(&err, "E001");
apep_error_set_message(&err, "bad token");
apep_error_set_location(&err, "input.txt", line, col);
apep_error_add_context(&err, line_text, line);
apep_error_print(&err, NULL);
```

**APEP advantages:**
- Clear visual hierarchy
- Automatic Unicode/color detection
- Consistent formatting
- Easy to extend with context

### APEP vs Log.c

Log.c is a minimal C logging library.

**Similarities:**
- Zero dependencies
- Simple API
- C library
- Minimal footprint

**Differences:**
- APEP: Diagnostic renderer (what to display)
- Log.c: Logging framework (when/where/what to log)
- APEP: Rich formatting with context
- Log.c: Simple log levels + sinks
- APEP: Hex dump support
- Log.c: No binary data support

**Use together:**
You can build Log.c-style logging on top of APEP for rich output.

### APEP vs spdlog

spdlog is a modern C++ logging framework.

**spdlog advantages:**
- Async logging with performance optimization
- Multiple sinks (console, file, syslog, etc.)
- Pattern formatting
- Performance metrics
- Thread-safe

**APEP advantages:**
- Zero dependencies (spdlog has many)
- C language (no C++ overhead)
- Hex dump diagnostics
- Localization system
- Simpler API
- Smaller binary

**When to use spdlog:**
- Performance-critical high-volume logging
- Complex logging infrastructure
- C++ projects
- Server applications

**When to use APEP:**
- Compiler/interpreter diagnostics
- User-facing error messages
- Low-dependency projects
- C-only codebases
- Rich context display

**Building a spdlog-style logger on APEP:**

```c
typedef struct {
    apep_options_t opts;
    FILE *file_sink;
    int min_level;
} apep_logger_t;

void apep_log_info(apep_logger_t *log, const char *tag, const char *msg) {
    if (APEP_LVL_INFO >= log->min_level) {
        apep_print_message(&log->opts, APEP_LVL_INFO, tag, msg);
        if (log->file_sink) {
            fprintf(log->file_sink, "[INFO][%s] %s\n", tag, msg);
        }
    }
}
```

### APEP vs Rust's miette/ariadne

These are Rust diagnostic libraries.

**Their focus:**
- Rust language diagnostics
- Rich error display similar to Rust compiler
- Can output C-compatible formats (JSON)

**Why APEP is needed:**
- C doesn't have equivalent standard library
- APEP brings similar patterns to C
- Smaller footprint than porting Rust code

**Compatibility:**
- APEP can export JSON for Rust tools
- Rust tools can consume APEP JSON output
- Can build Rust bindings to APEP

### APEP vs Custom Solutions

**Without APEP:**
- Developers write their own error formatting
- Inconsistent across projects
- Often missing context (colors, Unicode, terminal width)
- Difficult to maintain across platforms

**With APEP:**
- Consistent diagnostics
- Platform-agnostic code
- Automatic capability detection
- Easy to extend

---

## Feature Deep-Dive

### Text Diagnostics (Error Messages with Context)

```
error[E001]: unexpected token ')'
  -> input.c:5:10
      |
    4 | int main(void) {
    5 |   return (1+);
      |           ^^ here
```

**APEP:** Supports this natively with context lines
**printf:** Would need 50+ lines of custom code
**Log.c:** Not designed for this
**spdlog:** Possible with custom formatting
**miette/ariadne:** Primary use case (Rust only)

### Binary Diagnostics (Hex Dumps)

```
      0 1 2 3  4 5 6 7  8 9 A B  C D E F
    0 48 65 6c 6c 6f 20 57 6f 72 6c 64 21 00 00 00
    1 00 00 00 ...
          ^^--- error: invalid marker
```

**APEP:** Supports this
**printf:** Would need custom formatting
**All others:** Not designed for binary

### Localization

```c
// English
apep_error("E001", "file not found", "input.txt");

// Czech
apep_i18n_set_locale("cs");
apep_error("E001", "soubor nenalezen", "input.txt");
```

**APEP:** Built-in via `.loc` files
**printf:** Requires integration with gettext
**Log.c:** Not designed for localization
**spdlog:** No built-in system
**miette/ariadne:** No localization support

---

## Choosing APEP

APEP is the right choice if you need:

1. **Rust-style error messages in C**
   - Rich context and formatting
   - Beautiful terminal output
   - Graceful degradation

2. **Zero dependencies**
   - Embedded systems
   - Minimal C runtimes
   - Secure/constrained environments

3. **Hex dump diagnostics**
   - Binary protocol parsing
   - File format validation
   - Network packet analysis

4. **Localization**
   - International user bases
   - Multi-language support
   - Easy translation workflow

5. **Small binary footprint**
   - Embedded applications
   - Static linking
   - Minimal overhead

6. **Terminal capability detection**
   - Automatic color/Unicode support
   - Graceful terminal fallbacks
   - Deterministic CI output

---

## Migration Guide

### From printf to APEP

**Before:**

```c
apep_set_color_mode(APEP_COLOR_OFF);
```

Or set the `NO_COLOR` environment variable.

### Can I force ASCII-only output?
Yes:

```c
apep_set_unicode_mode(APEP_UNICODE_OFF);
```

### How do I change the terminal width?

```c
apep_options_t opt;
apep_options_default(&opt);
opt.width_override = 120;  // Fixed width
```

## Internationalization

### What languages are supported?
Currently: English and Czech. More languages can be easily added - see [locales/README.md](locales/README.md).

### How do I add a new language?
1. Copy `locales/en.loc` to `locales/<code>.loc`
2. Translate the values (keep keys unchanged)
3. Test with `apep_i18n_init("<code>", NULL)`

### Can I change locale at runtime?
Yes:

```c
apep_i18n_set_locale("cs");  // Switch to Czech
apep_i18n_set_locale("en");  // Switch to English
```

### How does auto-detection work?
APEP reads system locale settings:
- **Windows**: `GetUserDefaultLCID()`
- **Linux/macOS**: `LANG`, `LC_ALL` environment variables

## Integration

### How do I integrate APEP into my project?
**Makefile:**

```makefile
CFLAGS += -I/path/to/apep/include
LDFLAGS += -L/path/to/apep -lapep
```

**CMake:**

```cmake
add_subdirectory(apep)
target_link_libraries(myapp PRIVATE apep)
```

### Does APEP have any dependencies?
No. APEP is **zero-dependency**. It only uses standard C library and OS APIs.

### Can I use APEP in a commercial product?
Check the LICENSE file. APEP uses the AHPL license.

## Performance

### Is APEP fast?
Yes. Diagnostic rendering is typically <1ms. I18n lookup is O(1) with hash tables.

### Does APEP allocate memory?
APEP uses minimal stack allocation. Locale data is loaded once into heap memory.

### Is APEP thread-safe?
The diagnostic functions are thread-safe for rendering. However, i18n global state is not thread-safe. Initialize localization before creating threads.

## Troubleshooting

### Colors don't show in my terminal
- Check if `NO_COLOR` environment variable is set
- Try `apep_set_color_mode(APEP_COLOR_ON)` to force colors
- Verify your terminal supports ANSI colors

### Unicode characters show as garbage
- Ensure your terminal uses UTF-8 encoding
- Set `export LANG=en_US.UTF-8`
- Or use `apep_set_unicode_mode(APEP_UNICODE_OFF)` for ASCII

### Locale file not found
- Check that `locales/<code>.loc` exists
- Use `apep_i18n_set_locales_path("path/to/locales")`  to set custom path
- Verify file permissions

### Wrong terminal width detected

```c
apep_options_t opt;
apep_options_default(&opt);
opt.width_override = 80;  // Override detection
```

## Comparison

### APEP vs printf?
APEP provides structured, formatted diagnostics with source context. `printf` is for simple text output.

### APEP vs spdlog/log4c?
Those are logging frameworks. APEP is a diagnostic renderer. You can build a logger on top of APEP.

### APEP vs Rust's miette/ariadne?
APEP brings similar diagnostic styling to C. Those libraries are for Rust.

## Contributing

### Can I contribute?
Yes! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### How do I report a bug?
Open an issue on GitHub with:
- APEP version
- Platform (OS, compiler)
- Minimal reproduction code
- Expected vs actual output

### Can I add a new feature?
Please open an issue first to discuss the feature before implementing it.

## License

### What license does APEP use?
See the [LICENSE](LICENSE) file for details.