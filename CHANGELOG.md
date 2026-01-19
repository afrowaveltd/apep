# Changelog

All notable changes to APEP will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added - Major Feature Update 2026-01-19 🎉

#### JSON Output
- `apep_print_json_diagnostic()` - Export diagnostics in JSON format
- Structured output for IDE/CI/CD integration
- Full support for notes, locations, and severity

#### Severity Filtering
- `apep_set_min_severity()` - Control which diagnostics are shown
- `apep_get_min_severity()` - Query current filter level
- `apep_severity_passes_filter()` - Check if diagnostic passes filter

#### Diagnostic Buffering
- `apep_buffer_create/add/flush/clear/destroy()` - Buffer management
- Optional sorting by file/line location
- Batch processing of diagnostics

#### Color Schemes
- Multiple predefined schemes (default, dark, light, colorblind)
- `apep_set_color_scheme()` - Choose scheme
- `apep_set_custom_colors()` - Custom color palettes
- Accessibility improvements

#### Stack Trace
- `APEP_TRACE_BEGIN()` / `APEP_TRACE_END()` - Stack tracking
- `apep_stack_print()` - Display call stack
- Better debugging context

#### Suggestions & Diff
- `apep_print_text_diagnostic_with_suggestion()` - Show fixes
- "Did you mean?" style hints
- Before/after code display

#### Multi-Span Highlighting
- `apep_print_text_diagnostic_multi()` - Multiple highlights
- Labeled spans
- Type mismatch visualization

#### Performance Metrics
- `apep_perf_start/end()` - Timing functions
- Millisecond precision
- `APEP_PERF()` macros

#### Progress Reporting
- `apep_progress_start/update/done()` - Progress bars
- Animated for TTY, periodic for non-TTY
- Long-running operation feedback

#### Enhanced Assertions
- `APEP_ASSERT()` / `APEP_ASSERT_FMT()` - Rich assertions
- Automatic stack trace on failure
- Formatted messages

### Documentation
- New `docs/ADVANCED.md` - All new features explained
- Updated `screenshots/README.md` - Screenshot catalog
- Enhanced main documentation with examples
- New demo: `examples/new_features_demo.c`

### Added (Previous)
- Complete internationalization (i18n) system
- Auto-detect system locale on Windows, Linux, macOS
- English and Czech translations
- Hash table-based string lookup for O(1) performance
- Locale file format (.loc files)
- Runtime locale switching
- i18n demo programs

### Documentation
- Complete API reference (docs/API.md)
- Internationalization guide (docs/I18N.md)
- Capabilities detection guide (docs/CAPABILITIES.md)
- Logger integration guide (docs/LOGGER_INTEGRATION.md)
- Quick start guide (QUICKSTART.md)
- Locale files documentation (locales/README.md)

## [0.1.0] - Initial Release

### Added
- Rust-style text diagnostics with source context
- Binary hexdump diagnostics
- Six severity levels (trace, debug, info, warn, error, critical)
- Auto-detection of TTY, colors, unicode, terminal width
- Graceful degradation for minimal terminals
- Helper functions for common error patterns
- Zero external dependencies
- Cross-platform support (Windows, Linux, macOS)
- CMake and Makefile build systems
- Comprehensive examples
- CI/CD with GitHub Actions

### Core Features
- Text error rendering with caret/span highlighting
- Hex dump with highlighted byte spans
- Color and Unicode auto-detection
- Configurable output options
- Printf-style formatting
- Notes, hints, and help messages

[Unreleased]: https://github.com/afrowaveltd/apep/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/afrowaveltd/apep/releases/tag/v0.1.0