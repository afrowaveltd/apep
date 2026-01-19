# APEP (Afrowave Pretty Error Print)

[![CI](https://github.com/afrowaveltd/apep/workflows/CI/badge.svg)](https://github.com/afrowaveltd/apep/actions)
[![License: AHPL](https://img.shields.io/badge/License-AHPL-blue.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-0.1.0-green.svg)](CHANGELOG.md)

**Beautiful, truthful diagnostics for C projects**

APEP is a zero-dependency C library for printing Rust-style error messages, hexdumps, and structured logs. It adapts to your terminal automatically and degrades gracefully everywhere.

```c
error[E0001]: unexpected token ')'
  -> input.expr:1:4
      |
    1 | (1+)
      |    ^
  = hint: expected expression after '+' operator
```

Built with ❤️ by the Afrowave community for developers who care about user experience.

---

## What APEP does

APEP provides **pretty error rendering** for two common diagnostic domains:

### 1. Text diagnostics (source code, input files)

* Rust‑style caret diagnostics
* File or label name
* Line and column (1‑based)
* Context lines before and after the error
* Caret or span highlighting
* Optional notes, hints, or help messages

Typical use cases:

* language parsers
* configuration loaders
* input validators
* test failures

### 2. Binary diagnostics (buffers, files)

* Hexdump output with offsets
* Configurable context window around the error
* Highlighted byte span
* ASCII preview when available

Typical use cases:

* corrupted files
* checksum mismatches
* protocol parsing
* binary format validation

Binary data is **never printed raw**. Only text representations are used.

---

## Core principles

* **Truth over beauty**
  Diagnostics must never lie or guess.

* **Beauty when possible**
  When the output allows it, APEP renders rich, readable diagnostics.

* **Graceful degradation**
  If colors, Unicode, or width detection are unavailable, APEP falls back
  to plain ASCII without breaking layout.

* **Portability first**
  APEP targets Linux, FreeBSD, Windows, macOS, and low‑resource systems
  with minimal dependencies.

* **Deterministic output when needed**
  Fixed width, colors disabled, and stable formatting are supported
  for CI pipelines and logs.

---

## What APEP is *not*

APEP is intentionally **not**:

* a logging framework
* a test engine
* a UI toolkit
* a replacement for structured reports (JSON, AJIS, etc.)

APEP focuses on one task only:

> Make errors visible, precise, and understandable.

---

## Capabilities detection

APEP detects what the output environment can handle:

* TTY vs non‑TTY output
* color support (AUTO / ON / OFF)
* Unicode line drawing (AUTO / ON / OFF)
* terminal width (with safe fallback)

Host applications may override detection explicitly.

---

## Quick Start

### Using Makefile

```sh
make
./bin/apep_show_demo --plain
```

### Using CMake

```sh
mkdir build && cd build
cmake ..
cmake --build .
./apep_show_demo --plain
```

### Integration

**With make:**

```makefile
# In your project Makefile
APEP_DIR = path/to/apep
CFLAGS += -I$(APEP_DIR)/include
LDFLAGS += -L$(APEP_DIR) -lapep

# Build APEP first
$(APEP_DIR)/libapep.a:
	$(MAKE) -C $(APEP_DIR)
```

**With CMake:**

```cmake
add_subdirectory(apep)
target_link_libraries(your_app PRIVATE apep)
```

See [QUICKSTART.md](QUICKSTART.md) for code examples.

---

## Documentation

- **[QUICKSTART.md](QUICKSTART.md)** - Code examples and usage guide
- **[docs/API.md](docs/API.md)** - Complete API reference
- **[docs/I18N.md](docs/I18N.md)** - Internationalization and localization
- **[docs/CAPABILITIES.md](docs/CAPABILITIES.md)** - Feature overview
- **[docs/LOGGER_INTEGRATION.md](docs/LOGGER_INTEGRATION.md)** - Use APEP as logger backend
- **[COMPARISON.md](COMPARISON.md)** - APEP vs alternatives (printf, Log.c, Spdlog, etc.)
- **[IMPROVEMENTS.md](IMPROVEMENTS.md)** - Future enhancements
- **[CONTRIBUTING.md](CONTRIBUTING.md)** - How to contribute

---

## Use Cases

### Compiler/Interpreter Diagnostics
Perfect for language implementations that need clear error messages with source context.

### Application Logging
Use APEP as a terminal output backend for your logging framework (Serilog-style).

### Binary Protocol Debugging
Visualize packet data with highlighted spans and ASCII preview.

### Configuration Validation
Show users exactly where configuration errors occur.

See [LOGGER_INTEGRATION.md](docs/LOGGER_INTEGRATION.md) for a complete guide on building a universal logger on top of APEP.

---

## Features

✨ **Beautiful Output**
- Rust-style error messages
- Colored output (auto-detected)
- Unicode box drawing (when supported)
- ASCII fallback for minimal terminals

📊 **Structured Logging**
- 6 severity levels (trace → critical)
- Tagged messages
- Optional timestamps
- Pluggable output

🔍 **Diagnostics**
- Text errors with context lines
- Binary hexdumps with highlighting
- Multi-line context
- Notes, hints, and suggestions

🚀 **Developer Friendly**
- Helper macros for quick logging
- Printf-style formatting
- Global configuration
- Zero boilerplate

🌍 **Portable**
- Zero dependencies
- C11/GNU11 compatible
- Windows, Linux, macOS
- Minimal resource usage

---

## Capabilities Detection

APEP automatically detects:
- TTY vs non-TTY output
- Color support (respects NO_COLOR)
- Unicode capability
- Terminal width
- CI environment

Override detection explicitly when needed.

---

## License

See the LICENSE file.