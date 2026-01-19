# Frequently Asked Questions (FAQ)

## General

### What is APEP?
APEP (Afrowave Pretty Error Print) is a zero-dependency C library for printing Rust-style error messages, hexdumps, and structured logs.

### Why "APEP"?
APEP stands for **Afrowave Pretty Error Print**. It's also a reference to the ancient Egyptian deity Apep (Apophis), symbolizing chaos that must be tamed through clear, truthful diagnostics.

### Is APEP a logging framework?
No. APEP is a **diagnostic renderer**. It focuses solely on making errors and messages visible and understandable. You can build a logging framework on top of APEP (see [docs/LOGGER_INTEGRATION.md](docs/LOGGER_INTEGRATION.md)).

### Does APEP work on Windows?
Yes! APEP works on Windows (MSYS2/MinGW, MSVC), Linux, macOS, and FreeBSD.

## Usage

### Do I need to initialize APEP?
Not for basic usage. APEP uses sensible defaults. For localization, call `apep_i18n_init()`.

### Can I disable colors?
Yes:
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
