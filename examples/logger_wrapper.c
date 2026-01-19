/**
 * Logger Wrapper Example - How to use APEP as a universal logger backend
 *
 * This demonstrates building a Serilog-like logging system on top of APEP.
 */

#include <apep/apep_helpers.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// ============================================================================
// Logger API
// ============================================================================

typedef enum log_level
{
    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO = 2,
    LOG_WARN = 3,
    LOG_ERROR = 4,
    LOG_FATAL = 5
} log_level_t;

typedef struct logger
{
    const char *name;
    log_level_t min_level;
    apep_options_t apep_opts;
} logger_t;

// Create a new logger instance
logger_t *logger_create(const char *name, log_level_t min_level)
{
    logger_t *log = (logger_t *)malloc(sizeof(logger_t));
    if (!log)
        return NULL;

    log->name = strdup(name);
    log->min_level = min_level;

    // Initialize APEP with sensible defaults
    apep_options_default(&log->apep_opts);
    apep_set_global_options(&log->apep_opts);

    return log;
}

// Free logger resources
void logger_free(logger_t *log)
{
    if (log)
    {
        free((void *)log->name);
        free(log);
    }
}

// Set minimum log level
void logger_set_level(logger_t *log, log_level_t level)
{
    log->min_level = level;
}

// Map log level to APEP level
static apep_level_t map_level(log_level_t level)
{
    switch (level)
    {
    case LOG_TRACE:
        return APEP_LVL_TRACE;
    case LOG_DEBUG:
        return APEP_LVL_DEBUG;
    case LOG_INFO:
        return APEP_LVL_INFO;
    case LOG_WARN:
        return APEP_LVL_WARN;
    case LOG_ERROR:
        return APEP_LVL_ERROR;
    case LOG_FATAL:
        return APEP_LVL_CRITICAL;
    default:
        return APEP_LVL_INFO;
    }
}

// Core logging function
void logger_log(logger_t *log, log_level_t level, const char *fmt, ...)
{
    if (!log || level < log->min_level)
        return;

    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    apep_print_message(NULL, map_level(level), log->name, buffer);
}

// Convenience macros
#define LOG_TRACE_FMT(log, ...) logger_log(log, LOG_TRACE, __VA_ARGS__)
#define LOG_DEBUG_FMT(log, ...) logger_log(log, LOG_DEBUG, __VA_ARGS__)
#define LOG_INFO_FMT(log, ...) logger_log(log, LOG_INFO, __VA_ARGS__)
#define LOG_WARN_FMT(log, ...) logger_log(log, LOG_WARN, __VA_ARGS__)
#define LOG_ERROR_FMT(log, ...) logger_log(log, LOG_ERROR, __VA_ARGS__)
#define LOG_FATAL_FMT(log, ...) logger_log(log, LOG_FATAL, __VA_ARGS__)

// ============================================================================
// Advanced Features
// ============================================================================

// Log with source code context
void logger_error_source(logger_t *log, const char *file, const char *code,
                         int line, int col, size_t len, const char *msg)
{
    if (!log || LOG_ERROR < log->min_level)
        return;

    FILE *f = fopen(file, "r");
    if (!f)
    {
        logger_log(log, LOG_ERROR, "%s", msg);
        return;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = (char *)malloc((unsigned long)size + 1);
    if (!content)
    {
        fclose(f);
        return;
    }

    size_t read_size = fread(content, 1, (unsigned long)size, f);
    content[read_size] = '\0';
    fclose(f);

    apep_text_source_t src = apep_text_source_from_string(file, content);
    apep_print_text_diagnostic(NULL, APEP_SEV_ERROR, code, msg,
                               &src, (apep_loc_t){line, col}, len,
                               NULL, 0);

    free(content);
}

// Log binary data
void logger_hex(logger_t *log, log_level_t level, const char *label,
                const uint8_t *data, size_t size,
                size_t highlight_offset, size_t highlight_len)
{
    if (!log || level < log->min_level)
        return;

    apep_severity_t sev;
    switch (level)
    {
    case LOG_ERROR:
        sev = APEP_SEV_ERROR;
        break;
    case LOG_WARN:
        sev = APEP_SEV_WARN;
        break;
    case LOG_INFO:
        sev = APEP_SEV_NOTE;
        break;
    default:
        sev = APEP_SEV_NOTE;
        break;
    }

    apep_span_t span = {highlight_offset, highlight_len};
    apep_print_hex_diagnostic(NULL, sev, log->name, label, label,
                              data, size, span, NULL, 0);
}

// ============================================================================
// Configuration
// ============================================================================

void logger_configure_from_env(logger_t *log)
{
    if (!log)
        return;

    // Set log level from environment
    const char *level_str = getenv("LOG_LEVEL");
    if (level_str)
    {
        if (strcmp(level_str, "trace") == 0)
            log->min_level = LOG_TRACE;
        else if (strcmp(level_str, "debug") == 0)
            log->min_level = LOG_DEBUG;
        else if (strcmp(level_str, "info") == 0)
            log->min_level = LOG_INFO;
        else if (strcmp(level_str, "warn") == 0)
            log->min_level = LOG_WARN;
        else if (strcmp(level_str, "error") == 0)
            log->min_level = LOG_ERROR;
        else if (strcmp(level_str, "fatal") == 0)
            log->min_level = LOG_FATAL;
    }

    // Configure APEP options
    if (getenv("FORCE_COLOR"))
    {
        log->apep_opts.color = APEP_COLOR_ON;
    }
    else if (getenv("NO_COLOR"))
    {
        log->apep_opts.color = APEP_COLOR_OFF;
    }

    if (getenv("FORCE_ASCII"))
    {
        log->apep_opts.force_ascii = 1;
    }

    apep_set_global_options(&log->apep_opts);
}

// ============================================================================
// Demo Application
// ============================================================================

int main(void)
{
    printf("=== Logger Wrapper Demo ===\n\n");

    // Create logger instances for different components
    logger_t *app_log = logger_create("APP", LOG_INFO);
    logger_t *net_log = logger_create("NET", LOG_DEBUG);
    logger_t *db_log = logger_create("DB", LOG_WARN);

    // Configure from environment variables
    logger_configure_from_env(app_log);

    printf("1. Basic Logging\n");
    printf("----------------\n");
    LOG_INFO_FMT(app_log, "Application starting...");
    LOG_DEBUG_FMT(net_log, "Initializing network stack");
    LOG_INFO_FMT(net_log, "Listening on port %d", 8080);
    LOG_WARN_FMT(db_log, "Connection pool size low: %d/%d", 2, 10);
    LOG_ERROR_FMT(app_log, "Failed to load plugin: %s", "analytics.so");
    LOG_FATAL_FMT(app_log, "Out of memory!");
    printf("\n");

    printf("2. Structured Logging\n");
    printf("---------------------\n");
    LOG_INFO_FMT(app_log, "User logged in: user=%s, ip=%s, method=%s",
                 "alice", "192.168.1.100", "password");
    LOG_WARN_FMT(net_log, "Slow request: path=%s, duration=%dms, threshold=%dms",
                 "/api/users", 1500, 1000);
    printf("\n");

    printf("3. Source Code Diagnostics\n");
    printf("--------------------------\n");
    // Create a temporary source file for demo
    FILE *tmp = fopen("temp_error.txt", "w");
    if (tmp)
    {
        fprintf(tmp, "{\n");
        fprintf(tmp, "  \"name\": \"Alice\",\n");
        fprintf(tmp, "  \"age\": \"invalid\"\n");
        fprintf(tmp, "}\n");
        fclose(tmp);

        logger_error_source(app_log, "temp_error.txt", "E_TYPE",
                            3, 11, 9,
                            "expected number, got string");
        remove("temp_error.txt");
    }
    printf("\n");

    printf("4. Binary Data Logging\n");
    printf("----------------------\n");
    uint8_t packet[] = {
        0x47, 0x45, 0x54, 0x20, // GET
        0x2F, 0x61, 0x70, 0x69, // /api
        0x2F, 0x75, 0x73, 0x65, // /use
        0x72, 0x73, 0x00, 0x00  // rs..
    };
    logger_hex(net_log, LOG_INFO, "HTTP Request", packet, sizeof(packet), 0, 4);
    printf("\n");

    printf("5. Performance Test\n");
    printf("-------------------\n");
    LOG_INFO_FMT(app_log, "Starting performance test with %d iterations", 10000);

    // This would be too verbose, so we disable it
    logger_set_level(app_log, LOG_WARN);
    for (int i = 0; i < 10; i++)
    {
        LOG_DEBUG_FMT(app_log, "Iteration %d", i); // Won't print
    }
    logger_set_level(app_log, LOG_INFO);

    LOG_INFO_FMT(app_log, "Performance test completed");
    printf("\n");

    printf("6. Multi-Component Logging\n");
    printf("--------------------------\n");
    LOG_INFO_FMT(app_log, "Processing request #1234");
    LOG_DEBUG_FMT(net_log, "  ├─ Receiving data...");
    LOG_DEBUG_FMT(db_log, "  ├─ Querying database...");
    LOG_DEBUG_FMT(db_log, "  │  └─ Query: SELECT * FROM users WHERE id=?");
    LOG_DEBUG_FMT(app_log, "  ├─ Processing results...");
    LOG_INFO_FMT(app_log, "  └─ Request completed in 45ms");
    printf("\n");

    // Cleanup
    logger_free(app_log);
    logger_free(net_log);
    logger_free(db_log);

    printf("=== Demo Complete ===\n");
    return 0;
}
