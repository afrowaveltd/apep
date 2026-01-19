# APEP Capabilities Detection

APEP automatically detects terminal capabilities.

## What's Detected

- **TTY**: Is output a terminal?
- **Colors**: ANSI color support
- **Unicode**: UTF-8 box drawing characters
- **Width**: Terminal width in columns

## Auto-Detection

```c
apep_caps_t caps = apep_detect_caps(stderr, &opt);
```

## Manual Override

```c
apep_options_t opt;
apep_options_default(&opt);

// Force colors
opt.color = APEP_COLOR_ON;

// Force ASCII (no Unicode)
opt.unicode = APEP_UNICODE_OFF;

// Fixed width
opt.width_override = 80;
```

## Environment Variables

- \NO_COLOR\: If set, disables colors
- \TERM=dumb\: Disables colors and Unicode
- \LANG=*.UTF-8\: Enables Unicode

## Platform Differences

### Linux/macOS
- TTY: \isatty()\
- Width: \ioctl(TIOCGWINSZ)\
- Locale: \LANG\ environment variable

### Windows
- TTY: \GetConsoleMode()\
- Width: \GetConsoleScreenBufferInfo()\
- Locale: \GetUserDefaultLCID()\

## CI/CD

In CI environments (GitHub Actions, GitLab CI):
- Colors typically disabled (no TTY)
- Use \CLICOLOR_FORCE=1\ to enable

## Testing

For deterministic output:
```c
opt.color = APEP_COLOR_OFF;
opt.unicode = APEP_UNICODE_OFF;
opt.width_override = 80;
```

See [API.md](API.md) for apep_options_t details.
