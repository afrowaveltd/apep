# APEP Logger Integration

Build a full-featured logging system on top of APEP.

APEP is a **diagnostic renderer**, not a logging framework. This guide shows how to build logging frameworks on top of APEP's powerful formatting capabilities.

## Architecture

```
Your Application
    ↓
Logger Wrapper (your code)
    ↓
APEP (renders diagnostics)
    ↓
Output (console, file, network, etc.)
```

The logger wrapper handles:
- Severity filtering
- Multiple output targets ("sinks")
- Buffering and batching
- Timestamp/component tagging
- Thread safety (optional)

APEP handles:
- Beautiful formatting
- Color detection
- Terminal width adaptation
- Localization
- Unicode/ASCII fallback

## Simple Logger

```c
typedef struct {
    apep_options_t opts;
    apep_level_t min_level;
    FILE *logfile;
} logger_t;

void logger_init(logger_t *log, apep_level_t min) {
    apep_options_default(&log->opts);
    log->min_level = min;
    log->logfile = NULL;
}

void logger_info(logger_t *log, const char *tag, const char *msg) {
    if (APEP_LVL_INFO >= log->min_level) {
        apep_print_message(&log->opts, APEP_LVL_INFO, tag, msg);
        if (log->logfile) {
            fprintf(log->logfile, "[INFO][%s] %s\n", tag, msg);
        }
    }
}
```

## Features to Add

- Multiple sinks (console, file, syslog)
- Log filtering by tag/level
- Structured logging (key-value pairs)
- Log rotation
- Timestamps

## Global Logger Pattern

```c
static logger_t g_logger;

void log_init(void) {
    logger_init(&g_logger, APEP_LVL_DEBUG);
}

#define LOG_INFO(tag, msg) logger_info(&g_logger, tag, msg)
```

## Use Cases

- Application logging
- HTTP server request logs
- Debugging output
- Test framework diagnostics

See examples/ for complete implementations.