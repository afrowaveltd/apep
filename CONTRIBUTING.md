# Contributing to APEP

Thank you for your interest in contributing to APEP! This document outlines how to build, test, and contribute to the project.

## Quick Links

- [Building](#building-apep)
- [Testing](#testing)
- [Code Style](#code-style)
- [Adding Languages](#adding-a-new-language)
- [Pull Requests](#submitting-changes)

## Project Structure

```
apep/
├── src/           # Library source code
├── include/apep/  # Public headers
├── examples/      # Demo programs
├── docs/          # Documentation
├── locales/       # Translation files (.loc)
├── cmake/         # CMake configuration
└── Makefile       # Build system
```

## Building APEP

### Prerequisites

- **Windows**: MinGW-w64 or MSVC 14.0+
- **Linux**: GCC 7+ or Clang 5+
- **macOS**: Xcode Command Line Tools
- **CMake**: 3.10+ (optional)

### With Makefile

```bash
make              # Build library
make examples     # Build demos
make test         # Run tests
make install      # Install (Linux/macOS)
make clean        # Clean build
```

### With CMake

```bash
mkdir build && cd build
cmake ..
cmake --build .
cmake --install .
```

### Windows (MinGW)

```bash
mingw32-make
mingw32-make examples
```

## Running Examples

```bash
./bin/show_demo              # Feature showcase
./bin/show_demo --plain      # ASCII only
./bin/helpers_demo           # Helper macros
./bin/hex_error_demo         # Binary diagnostics
./bin/i18n_demo --lang cs    # Czech language
```

See [examples/README.md](examples/README.md) for details.

## Code Style

### Naming
- Functions: `apep_*`
- Types: `apep_*_t`
- Constants: `APEP_*`

### Formatting
- Indentation: 4 spaces
- Line length: 100 chars (soft limit)
- Braces: K&R style

### Example

```c
void apep_function(int param) {
    if (condition) {
        // code
    }
}
```

## Adding a New Language

1. Copy `locales/en.loc` to `locales/<code>.loc`
2. Translate values (keep keys unchanged)
3. Test: `./bin/i18n_demo --lang <code>`
4. Submit PR

See [locales/README.md](locales/README.md) for details.

## Testing

```bash
make test              # Run all tests
./bin/show_demo        # Manual test
valgrind ./bin/show_demo  # Memory check (Linux)
```

### Test Checklist

- [ ] Builds on Windows (MinGW/MSVC)
- [ ] Builds on Linux (GCC/Clang)
- [ ] Examples run without crashes
- [ ] No memory leaks (valgrind)
- [ ] Translations work

## Submitting Changes

1. Fork the repository
2. Create a branch: `git checkout -b feature/my-feature`
3. Make changes
4. Test thoroughly
5. Commit: `git commit -m "feat: add feature"`
6. Push: `git push origin feature/my-feature`
7. Create Pull Request

### Commit Messages

```
feat: add JSON output support
fix: correct Unicode detection on Windows
docs: update API reference
test: add i18n test cases
```

## Reporting Issues

Include:
- OS and version
- Compiler and version
- Minimal reproduction code
- Expected vs actual output

## Questions?

See [FAQ.md](FAQ.md) or open an issue.

Thank you for contributing! 🎉