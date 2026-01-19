#include "../include/apep/apep_helpers.h"
#include "../include/apep/apep_i18n.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ----------------------------
Global options management
---------------------------- */

static apep_options_t g_global_options;
static int g_global_options_set = 0;

void apep_set_global_options(const apep_options_t *opt)
{
    if (opt)
    {
        g_global_options = *opt;
        g_global_options_set = 1;
    }
}

const apep_options_t *apep_get_global_options(void)
{
    if (!g_global_options_set)
    {
        apep_options_default(&g_global_options);
        g_global_options_set = 1;
    }
    return &g_global_options;
}

void apep_reset_global_options(void)
{
    apep_options_default(&g_global_options);
    g_global_options_set = 1;
}

/* ----------------------------
Helper functions
---------------------------- */

void apep_error_simple(
    const apep_options_t *opt,
    const char *code,
    const char *message,
    const char *hint)
{
    const apep_options_t *o = opt ? opt : apep_get_global_options();
    FILE *out = o->out ? o->out : stderr;

    /* Print error header */
    fprintf(out, "%s", _("error"));
    if (code && code[0])
    {
        fprintf(out, "[%s]", code);
    }
    fprintf(out, ": %s\n", message ? message : _("unknown error"));

    /* Print hint if provided */
    if (hint && hint[0])
    {
        fprintf(out, "  = %s: %s\n", _("hint"), hint);
    }
}

void apep_error_file(
    const apep_options_t *opt,
    const char *filename,
    const char *operation,
    const char *reason)
{
    const apep_options_t *o = opt ? opt : apep_get_global_options();
    char msg[512];

    const char *fname = filename ? filename : _("<unknown>");
    const char *op = operation ? operation : _("access");
    const char *rsn = reason ? reason : _("unknown error");

    snprintf(msg, sizeof(msg), _("failed to %s file '%s': %s"), op, fname, rsn);

    apep_error_simple(o, "E_FILE", msg, NULL);
}

void apep_error_assert(
    const apep_options_t *opt,
    const char *expr,
    const char *file,
    int line)
{
    const apep_options_t *o = opt ? opt : apep_get_global_options();
    char msg[512];

    snprintf(msg, sizeof(msg), _("assertion failed: %s"), expr ? expr : "");

    char hint[256];
    snprintf(hint, sizeof(hint), _("at %s:%d"), file ? file : _("<unknown>"), line);

    apep_error_simple(o, "E_ASSERT", msg, hint);
}

void apep_error_unknown_identifier(
    const apep_options_t *opt,
    const char *unknown,
    const char *suggestion,
    const apep_text_source_t *src,
    apep_loc_t loc)
{
    const apep_options_t *o = opt ? opt : apep_get_global_options();

    char msg[256];
    snprintf(msg, sizeof(msg), _("unknown identifier '%s'"), unknown ? unknown : "");

    apep_note_t note;
    const apep_note_t *notes = NULL;
    size_t note_count = 0;

    if (suggestion && suggestion[0])
    {
        char hint_msg[256];
        snprintf(hint_msg, sizeof(hint_msg), _("did you mean '%s'?"), suggestion);

        note.kind = _("hint");
        note.message = hint_msg;
        notes = &note;
        note_count = 1;

        apep_print_text_diagnostic(
            o, APEP_SEV_ERROR, "E_UNKNOWN",
            msg, src, loc, 0, notes, note_count);
    }
    else
    {
        apep_print_text_diagnostic(
            o, APEP_SEV_ERROR, "E_UNKNOWN",
            msg, src, loc, 0, NULL, 0);
    }
}

/* ----------------------------
Formatted variants
---------------------------- */

void apep_print_message_fmt(
    const apep_options_t *opt,
    apep_level_t lvl,
    const char *tag,
    const char *fmt,
    ...)
{
    char buffer[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    apep_print_message(opt, lvl, tag, buffer);
}

void apep_error_simple_fmt(
    const apep_options_t *opt,
    const char *code,
    const char *fmt,
    ...)
{
    char buffer[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    apep_error_simple(opt, code, buffer, NULL);
}
