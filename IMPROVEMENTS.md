# Future Improvements

This document tracks planned enhancements for APEP.

## High Priority

### Thread Safety
- [ ] Thread-local storage for i18n state
- [ ] Mutex-protected global options
- [ ] Thread-safe diagnostic rendering

### Locale System Enhancements
- [ ] Plural forms support (e.g., "1 error" vs "2 errors")
- [ ] Message catalogs (GNU gettext `.mo` compatibility)
- [ ] Lazy locale loading
- [ ] Locale-specific number formatting
- [ ] Date/time formatting per locale

### Performance
- [ ] Benchmark suite
- [ ] Memory pool for frequent allocations
- [ ] Optional static allocation mode
- [ ] Zero-copy string operations where possible

## Medium Priority

### Diagnostic Features
- [ ] Multi-file diagnostics (showing errors across multiple files)
- [ ] Diff-style output for "expected vs actual" errors
- [ ] Interactive error browser (TUI mode)
- [ ] Syntax highlighting for code snippets
- [ ] Diagnostic templates system

### Output Targets
- [ ] HTML output with CSS styling
- [ ] JSON/structured output for tools
- [ ] Markdown output
- [ ] Terminal hyperlinks (OSC 8)

### Platform Support
- [ ] WASM/Emscripten support
- [ ] Android NDK support
- [ ] iOS support
- [ ] Embedded systems (no malloc mode)

## Low Priority

### Developer Experience
- [ ] Python bindings
- [ ] Rust bindings
- [ ] Language server protocol (LSP) integration
- [ ] VS Code extension for `.loc` files
- [ ] Online playground/demo

### Documentation
- [ ] Video tutorials
- [ ] More real-world examples
- [ ] Integration guides for popular frameworks
- [ ] Localization for documentation itself

### Testing
- [ ] Fuzzing with AFL/libFuzzer
- [ ] Visual regression tests for output
- [ ] Cross-platform CI matrix expansion
- [ ] Performance regression tests

## Ideas / Research

### Advanced Features
- [ ] Custom diagnostic renderers (plugins)
- [ ] Remote diagnostic collection
- [ ] Diagnostic aggregation/deduplication
- [ ] Machine learning-based suggestion system
- [ ] Integration with crash reporters

### Accessibility
- [ ] Screen reader optimizations
- [ ] High-contrast themes
- [ ] Configurable fonts (Braille terminals)
- [ ] Audio feedback for diagnostics

## Won't Implement

These features are explicitly out of scope:

- ❌ GUI toolkit integration (APEP is terminal-focused)
- ❌ Log aggregation/search (use dedicated tools like ELK)
- ❌ Network protocols (use syslog, etc.)
- ❌ Database backends (APEP is for human output, not storage)

## Contributing

Have an idea? Please:
1. Check if it's already listed here
2. Open an issue to discuss it
3. Submit a PR with your implementation

See [CONTRIBUTING.md](CONTRIBUTING.md) for details.
