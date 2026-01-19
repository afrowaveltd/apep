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

#ifdef __cplusplus
}
#endif

#endif /* APEP_HELPERS_H */
