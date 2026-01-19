#ifndef APEP_H
#define APEP_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* ----------------------------
    Version
    ---------------------------- */

#define APEP_VERSION_MAJOR 0
#define APEP_VERSION_MINOR 1
#define APEP_VERSION_PATCH 0

    /* Forward declaration so prototypes can reference options early. */
    typedef struct apep_options apep_options_t;

    /* ----------------------------
    Capabilities & options
    ---------------------------- */

    typedef enum apep_level
    {
        APEP_LVL_TRACE = 0,
        APEP_LVL_DEBUG,
        APEP_LVL_INFO,
        APEP_LVL_WARN,
        APEP_LVL_ERROR,
        APEP_LVL_CRITICAL
    } apep_level_t;

    const char *apep_level_name(apep_level_t lvl);

    void apep_print_message(
        const apep_options_t *opt,
        apep_level_t lvl,
        const char *tag,    /* optional, e.g. "NET", "IO" */
        const char *message /* required */
    );

    typedef enum apep_color_mode
    {
        APEP_COLOR_AUTO = 0,
        APEP_COLOR_OFF = 1,
        APEP_COLOR_ON = 2
    } apep_color_mode_t;

    typedef enum apep_unicode_mode
    {
        APEP_UNICODE_AUTO = 0,
        APEP_UNICODE_OFF = 1,
        APEP_UNICODE_ON = 2
    } apep_unicode_mode_t;

    typedef enum apep_style
    {
        APEP_STYLE_COMPACT = 0,
        APEP_STYLE_FULL = 1
    } apep_style_t;

    typedef struct apep_caps
    {
        int is_tty;  /* 1 if output is a terminal */
        int color;   /* 1 if colors enabled */
        int unicode; /* 1 if unicode line art allowed */
        int width;   /* terminal width, fallback 80 */
    } apep_caps_t;

    typedef struct apep_options
    {
        FILE *out;                   /* where to print (stderr recommended for errors) */
        apep_style_t style;          /* compact/full */
        apep_color_mode_t color;     /* auto/on/off */
        apep_unicode_mode_t unicode; /* auto/on/off */
        int width_override;          /* <=0 means auto */
        int context_lines;           /* for text errors (default 2) */
        int hex_bytes_per_line;      /* default 16 */
        int hex_context_bytes;       /* default 32 or 64 */

        /* Hard overrides (useful for demos / CLI flags). */
        int force_no_color; /* if 1, disable color regardless of TTY/CI/NO_COLOR */
        int force_ascii;    /* if 1, force ASCII-only (no unicode arrows etc.) */
    } apep_options_t;

    /* Fill defaults (safe, portable) */
    void apep_options_default(apep_options_t *opt);

    /* Detect capabilities for current output stream. */
    apep_caps_t apep_detect_caps(FILE *out, const apep_options_t *opt);

    /* ----------------------------
    Diagnostics model
    ---------------------------- */

    typedef struct apep_loc
    {
        int line; /* 1-based */
        int col;  /* 1-based */
    } apep_loc_t;

    typedef struct apep_span
    {
        size_t offset; /* for binary: absolute offset */
        size_t length; /* bytes */
    } apep_span_t;

    typedef struct apep_range
    {
        apep_loc_t start; /* inclusive */
        apep_loc_t end;   /* inclusive or end-exclusive is a policy; we will document v1 */
    } apep_range_t;

typedef enum apep_severity
{
    APEP_SEV_ERROR = 0,
    APEP_SEV_WARN = 1,
    APEP_SEV_NOTE = 2
} apep_severity_t;

typedef struct apep_note
{
    const char *kind;    /* "note", "hint", "help", ... */
    const char *message; /* single-line preferred */
} apep_note_t;

/* ----------------------------
Text source abstraction
---------------------------- */

/* Callback returns:
- 1 if line exists, and sets *line_ptr and *line_len
- 0 if line does not exist
The returned pointer must remain valid until the next callback call. */

typedef int (*apep_line_getter_fn)(
    void *user,
    int line_no_1based,
    const char **line_ptr,
    size_t *line_len);

typedef struct apep_text_source
{
    const char *name; /* filename or label */
    apep_line_getter_fn get_line;
    void *user;
} apep_text_source_t;

/* Helper: create text source from a single in-memory UTF-8 string. */
apep_text_source_t apep_text_source_from_string(const char *name, const char *text);

/* ----------------------------
Pretty printers
---------------------------- */

/* Pretty print a text error with caret/range.
- code may be NULL (e.g. "E_SYNTAX")
- message must be non-NULL
*/
void apep_print_text_diagnostic(
    const apep_options_t *opt,
    apep_severity_t sev,
    const char *code,
    const char *message,
    const apep_text_source_t *src,
    apep_loc_t loc,
    int span_len_cols, /* 0 => caret only */
    const apep_note_t *notes,
    size_t notes_count);

/* Pretty print a binary diagnostic with hexdump and highlighted span.
- data may be NULL if not available (then prints metadata only)
*/
void apep_print_hex_diagnostic(
    const apep_options_t *opt,
    apep_severity_t sev,
    const char *code,
    const char *message,
    const char *blob_name,
    const uint8_t *data,
    size_t data_size,
    apep_span_t span,
    const apep_note_t *notes,
    size_t notes_count);

/* ----------------------------
Utility (public, minimal)
---------------------------- */

const char *apep_severity_name(apep_severity_t sev);

#ifdef __cplusplus
}
#endif

#endif /* APEP_H */