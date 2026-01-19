#ifndef APEP_HELPERS_H
#define APEP_HELPERS_H

#include "apep.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* ----------------------------
    Convenience macros for quick usage
    ---------------------------- */

/* Quick logging macros - use global defaults */
#define APEP_LOG_TRACE(tag, msg) \
    apep_print_message(NULL, APEP_LVL_TRACE, tag, msg)

#define APEP_LOG_DEBUG(tag, msg) \
    apep_print_message(NULL, APEP_LVL_DEBUG, tag, msg)

#define APEP_LOG_INFO(tag, msg) \
    apep_print_message(NULL, APEP_LVL_INFO, tag, msg)

#define APEP_LOG_WARN(tag, msg) \
    apep_print_message(NULL, APEP_LVL_WARN, tag, msg)

#define APEP_LOG_ERROR(tag, msg) \
    apep_print_message(NULL, APEP_LVL_ERROR, tag, msg)

#define APEP_LOG_CRITICAL(tag, msg) \
    apep_print_message(NULL, APEP_LVL_CRITICAL, tag, msg)

/* Conditional debug logging (compiled out in release builds) */
#ifndef NDEBUG
#define APEP_DEBUG(tag, msg) APEP_LOG_DEBUG(tag, msg)
#else
#define APEP_DEBUG(tag, msg) ((void)0)
#endif

    /* ----------------------------
    Helper functions for common patterns
    ---------------------------- */

    /* Simple error without source context */
    void apep_error_simple(
        const apep_options_t *opt,
        const char *code,
        const char *message,
        const char *hint);

    /* File I/O error helper */
    void apep_error_file(
        const apep_options_t *opt,
        const char *filename,
        const char *operation, /* "open", "read", "write", "close" */
        const char *reason);   /* NULL to use generic message */

    /* Assert failure helper */
    void apep_error_assert(
        const apep_options_t *opt,
        const char *expr,
        const char *file,
        int line);

    /* Unknown identifier with suggestions */
    void apep_error_unknown_identifier(
        const apep_options_t *opt,
        const char *unknown,
        const char *suggestion, /* NULL if no suggestion */
        const apep_text_source_t *src,
        apep_loc_t loc);

    /* ----------------------------
    Formatted message variants
    ---------------------------- */

    void apep_print_message_fmt(
        const apep_options_t *opt,
        apep_level_t lvl,
        const char *tag,
        const char *fmt,
        ...)
#if defined(__GNUC__) || defined(__clang__)
        __attribute__((format(printf, 4, 5)))
#endif
        ;

    void apep_error_simple_fmt(
        const apep_options_t *opt,
        const char *code,
        const char *fmt,
        ...)
#if defined(__GNUC__) || defined(__clang__)
        __attribute__((format(printf, 3, 4)))
#endif
        ;

    /* ----------------------------
    Global options management
    ---------------------------- */

    /* Set global default options (used when opt=NULL in other calls) */
    void apep_set_global_options(const apep_options_t *opt);

    /* Get current global options (returns defaults if not set) */
    const apep_options_t *apep_get_global_options(void);

    /* Reset global options to library defaults */
    void apep_reset_global_options(void);

    /* ----------------------------
    JSON Output
    ---------------------------- */

    typedef enum apep_output_format
    {
        APEP_FORMAT_PRETTY = 0, /* Standard pretty output */
        APEP_FORMAT_JSON = 1    /* JSON structured output */
    } apep_output_format_t;

    /* Print diagnostic in JSON format */
    void apep_print_json_diagnostic(
        FILE *out,
        apep_severity_t sev,
        const char *code,
        const char *message,
        const char *file,
        int line,
        int col,
        int span_len,
        const apep_note_t *notes,
        size_t notes_count);

    /* ----------------------------
    Severity Filtering
    ---------------------------- */

    /* Set minimum severity level (messages below this are suppressed) */
    void apep_set_min_severity(apep_severity_t min_sev);

    /* Get current minimum severity */
    apep_severity_t apep_get_min_severity(void);

    /* Check if severity passes filter */
    int apep_severity_passes_filter(apep_severity_t sev);

    /* ----------------------------
    Diagnostic Buffering/Batching
    ---------------------------- */

    typedef struct apep_diagnostic_buffer apep_diagnostic_buffer_t;

    /* Create a new diagnostic buffer */
    apep_diagnostic_buffer_t *apep_buffer_create(void);

    /* Add a diagnostic to the buffer */
    void apep_buffer_add(
        apep_diagnostic_buffer_t *buf,
        apep_severity_t sev,
        const char *code,
        const char *message,
        const char *file,
        int line,
        int col);

    /* Flush buffer (print all diagnostics, optionally sorted) */
    void apep_buffer_flush(
        apep_diagnostic_buffer_t *buf,
        const apep_options_t *opt,
        int sort_by_location); /* 1 = sort by file/line, 0 = keep order */

    /* Clear buffer without printing */
    void apep_buffer_clear(apep_diagnostic_buffer_t *buf);

    /* Destroy buffer */
    void apep_buffer_destroy(apep_diagnostic_buffer_t *buf);

    /* Get diagnostic count in buffer */
    size_t apep_buffer_count(const apep_diagnostic_buffer_t *buf);

    /* ----------------------------
    Color Schemes
    ---------------------------- */

    typedef enum apep_color_scheme
    {
        APEP_SCHEME_DEFAULT = 0,
        APEP_SCHEME_DARK = 1,
        APEP_SCHEME_LIGHT = 2,
        APEP_SCHEME_COLORBLIND = 3,
        APEP_SCHEME_CUSTOM = 4
    } apep_color_scheme_t;

    typedef struct apep_custom_colors
    {
        const char *error;
        const char *warning;
        const char *note;
        const char *highlight;
        const char *caret;
        const char *label;
        const char *dim;
    } apep_custom_colors_t;

    /* Set color scheme */
    void apep_set_color_scheme(apep_color_scheme_t scheme);

    /* Set custom colors (ANSI codes) */
    void apep_set_custom_colors(const apep_custom_colors_t *colors);

    /* Get current color scheme */
    apep_color_scheme_t apep_get_color_scheme(void);

    /* ----------------------------
    Stack Trace / Debug Context
    ---------------------------- */

#define APEP_MAX_STACK_DEPTH 32

    typedef struct apep_stack_frame
    {
        const char *function;
        const char *file;
        int line;
    } apep_stack_frame_t;

    /* Push a stack frame (use APEP_TRACE macro) */
    void apep_stack_push(const char *func, const char *file, int line);

    /* Pop a stack frame */
    void apep_stack_pop(void);

    /* Print current stack trace */
    void apep_stack_print(const apep_options_t *opt);

    /* Clear stack trace */
    void apep_stack_clear(void);

    /* RAII-style macro for automatic stack tracking */
#define APEP_TRACE()                                         \
    apep_stack_push(__func__, __FILE__, __LINE__);           \
    struct apep_trace_guard_##__LINE__                       \
    {                                                        \
        ~apep_trace_guard_##__LINE__() { apep_stack_pop(); } \
    } apep_trace_guard_instance_##__LINE__

    /* C-compatible version (manual cleanup required) */
#define APEP_TRACE_BEGIN() apep_stack_push(__func__, __FILE__, __LINE__)
#define APEP_TRACE_END() apep_stack_pop()

    /* ----------------------------
    Suggestions / Diff
    ---------------------------- */

    typedef struct apep_suggestion
    {
        const char *label;      /* "did you mean?", "try this instead", etc. */
        const char *code;       /* suggested code */
        apep_loc_t loc;         /* where to apply */
        int replacement_length; /* how many chars to replace (0 = insert) */
    } apep_suggestion_t;

    /* Print diagnostic with suggestion */
    void apep_print_text_diagnostic_with_suggestion(
        const apep_options_t *opt,
        apep_severity_t sev,
        const char *code,
        const char *message,
        const apep_text_source_t *src,
        apep_loc_t loc,
        int span_len_cols,
        const apep_note_t *notes,
        size_t notes_count,
        const apep_suggestion_t *suggestion);

    /* ----------------------------
    Multi-span Highlighting
    ---------------------------- */

    typedef struct apep_text_span
    {
        apep_loc_t loc;
        int length;
        const char *label; /* optional label for this span */
    } apep_text_span_t;

    /* Print diagnostic with multiple highlighted spans */
    void apep_print_text_diagnostic_multi(
        const apep_options_t *opt,
        apep_severity_t sev,
        const char *code,
        const char *message,
        const apep_text_source_t *src,
        const apep_text_span_t *spans,
        size_t spans_count,
        const apep_note_t *notes,
        size_t notes_count);

    /* ----------------------------
    Performance Metrics
    ---------------------------- */

    typedef struct apep_perf_timer apep_perf_timer_t;

    /* Start a performance timer */
    apep_perf_timer_t *apep_perf_start(const char *label);

    /* End timer and print result */
    void apep_perf_end(apep_perf_timer_t *timer, const apep_options_t *opt);

    /* Convenience macro */
#define APEP_PERF(label) apep_perf_timer_t *apep_timer_##label = apep_perf_start(#label)
#define APEP_PERF_END(label) apep_perf_end(apep_timer_##label, NULL)

    /* ----------------------------
    Progress Reporting
    ---------------------------- */

    typedef struct apep_progress apep_progress_t;

    /* Start a progress bar */
    apep_progress_t *apep_progress_start(
        const apep_options_t *opt,
        const char *label,
        size_t total);

    /* Update progress */
    void apep_progress_update(apep_progress_t *prog, size_t current);

    /* Finish progress */
    void apep_progress_done(apep_progress_t *prog);

    /* ----------------------------
    Enhanced Assertions
    ---------------------------- */

/* Assert with rich error message */
#define APEP_ASSERT(cond, msg)                                            \
    do                                                                    \
    {                                                                     \
        if (!(cond))                                                      \
        {                                                                 \
            apep_assert_failed(#cond, msg, __FILE__, __LINE__, __func__); \
            abort();                                                      \
        }                                                                 \
    } while (0)

/* Assert with formatted message */
#define APEP_ASSERT_FMT(cond, fmt, ...)                                 \
    do                                                                  \
    {                                                                   \
        if (!(cond))                                                    \
        {                                                               \
            apep_assert_failed_fmt(#cond, __FILE__, __LINE__, __func__, \
                                   fmt, __VA_ARGS__);                   \
            abort();                                                    \
        }                                                               \
    } while (0)

    /* Internal: called by APEP_ASSERT */
    void apep_assert_failed(
        const char *expr,
        const char *msg,
        const char *file,
        int line,
        const char *func);

    void apep_assert_failed_fmt(
        const char *expr,
        const char *file,
        int line,
        const char *func,
        const char *fmt,
        ...)
#if defined(__GNUC__) || defined(__clang__)
        __attribute__((format(printf, 5, 6)))
#endif
        ;

#ifdef __cplusplus
}
#endif

#endif /* APEP_HELPERS_H */
