#include "../include/apep/apep.h"
#include "../include/apep/apep_i18n.h"
#include "apep_internal.h"

#include <string.h>

/* clamp helpers */
static int apep_clamp_int(int v, int lo, int hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

static void apep_print_notes(FILE *out, const apep_caps_t *caps, const apep_note_t *notes, size_t notes_count)
{
    for (size_t i = 0; i < notes_count; i++)
    {
        const char *k = (notes[i].kind && notes[i].kind[0]) ? notes[i].kind : _("note");
        const char *m = notes[i].message ? notes[i].message : "";

        fputs("  = ", out);

        apep_color_begin(out, caps, APEP_CR_LABEL);
        fputs(k, out);
        apep_color_end(out, caps);

        fputs(": ", out);
        fputs(m, out);
        fputc('\n', out);
    }
}

static void apep_print_gutter_line(FILE *out, int line_no, const char *bar, const char *line, size_t len)
{
    /* Right-align line numbers to 4 columns (good enough for v1) */
    fprintf(out, " %4d %s ", line_no, bar);
    fwrite(line, 1, len, out);
    fputc('\n', out);
}

static void apep_print_gutter_empty(FILE *out, const char *bar)
{
    fprintf(out, "      %s\n", bar);
}

static void apep_print_caret_line(
    FILE *out,
    const apep_caps_t *caps,
    const char *bar,
    int col_1based,
    int span_len_cols)
{
    /* Render: spaces then ^ or ^^^^ */
    int col = (col_1based <= 0) ? 1 : col_1based;

    fprintf(out, "      %s ", bar);

    /* spaces before caret */
    for (int i = 1; i < col; i++)
        fputc(' ', out);

    /* Color the caret for better visibility */
    apep_color_begin(out, caps, APEP_CR_CARET);

    if (span_len_cols <= 1)
    {
        fputc('^', out);
    }
    else
    {
        for (int i = 0; i < span_len_cols; i++)
            fputc('^', out);
    }

    apep_color_end(out, caps);
    fputc('\n', out);
}

void apep_print_text_diagnostic(
    const apep_options_t *opt_in,
    apep_severity_t sev,
    const char *code,
    const char *message,
    const apep_text_source_t *src,
    apep_loc_t loc,
    int span_len_cols,
    const apep_note_t *notes,
    size_t notes_count)
{
    apep_options_t def;
    const apep_options_t *opt = opt_in;
    if (!opt)
    {
        apep_options_default(&def);
        opt = &def;
    }

    FILE *out = opt->out ? opt->out : stderr;

    /* Detect output capabilities and choose ASCII/Unicode framing */
    apep_caps_t caps = apep_detect_caps(out, opt);

    const char *bar = caps.unicode ? "│" : "|";
    const char *arrow = caps.unicode ? "→" : "->";

    /* Header line */
    apep_color_role_t role =
        (sev == APEP_SEV_ERROR) ? APEP_CR_SEV_ERROR : (sev == APEP_SEV_WARN) ? APEP_CR_SEV_WARN
                                                                             : APEP_CR_SEV_NOTE;

    apep_color_begin(out, &caps, role);
    fputs(apep_severity_name(sev), out);
    apep_color_end(out, &caps);

    if (code && code[0])
    {
        fputc('[', out);
        apep_color_begin(out, &caps, APEP_CR_LABEL);
        fputs(code, out);
        apep_color_end(out, &caps);
        fputc(']', out);
    }

    fputs(": ", out);
    fputs(message ? message : "", out);
    fputc('\n', out);

    /* Location line */
    const char *name = (src && src->name && src->name[0]) ? src->name : _("<input>");
    int line = (loc.line <= 0) ? 1 : loc.line;
    int col = (loc.col <= 0) ? 1 : loc.col;

    apep_color_begin(out, &caps, APEP_CR_DIM);
    fprintf(out, "  %s %s:%d:%d\n", arrow, name, line, col);
    apep_color_end(out, &caps);

    /* If we have no source, only print notes and return */
    if (!src || !src->get_line)
    {
        apep_print_notes(out, &caps, notes, notes_count);

        return;
    }

    int ctx = opt->context_lines;
    if (ctx < 0)
        ctx = 0;
    if (ctx > 10)
        ctx = 10; /* keep v1 sane */

    int from = line - ctx;
    int to = line + ctx;
    if (from < 1)
        from = 1;

    /* Print separator gutter line */
    apep_print_gutter_empty(out, bar);

    /* Print context lines that exist */
    for (int ln = from; ln <= to; ln++)
    {
        const char *line_ptr = NULL;
        size_t line_len = 0;

        int ok = src->get_line(src->user, ln, &line_ptr, &line_len);
        if (!ok)
        {
            /* If the requested line doesn't exist, stop after the error line range,
               but keep behavior stable: we break only if we're past the target line. */
            if (ln > line)
                break;
            continue;
        }

        /* For safety: if col points beyond line length, still print caret at end+1 */
        if (ln == line)
        {
            int max_col = (int)line_len + 1;
            col = apep_clamp_int(col, 1, (max_col < 1 ? 1 : max_col));
        }

        apep_print_gutter_line(out, ln, bar, line_ptr, line_len);

        if (ln == line)
        {
            int span = span_len_cols;
            if (span < 0)
                span = 0;
            if (span > 200)
                span = 200; /* avoid silly output */

            apep_print_caret_line(out, &caps, bar, col, span);
        }
    }

    /* Notes */
    apep_print_notes(out, &caps, notes, notes_count);
}
void apep_print_message(
    const apep_options_t *opt,
    apep_level_t lvl,
    const char *tag,
    const char *message)
{
    FILE *out = (opt && opt->out) ? opt->out : stderr;

    apep_caps_t caps = apep_detect_caps(out, opt);

    /* Map level -> color role */
    apep_color_role_t role = APEP_CR_LVL_INFO;
    switch (lvl)
    {
    case APEP_LVL_TRACE:
        role = APEP_CR_LVL_TRACE;
        break;
    case APEP_LVL_DEBUG:
        role = APEP_CR_LVL_DEBUG;
        break;
    case APEP_LVL_INFO:
        role = APEP_CR_LVL_INFO;
        break;
    case APEP_LVL_WARN:
        role = APEP_CR_LVL_WARN;
        break;
    case APEP_LVL_ERROR:
        role = APEP_CR_LVL_ERROR;
        break;
    case APEP_LVL_CRITICAL:
        role = APEP_CR_LVL_CRITICAL;
        break;
    default:
        role = APEP_CR_LVL_INFO;
        break;
    }

    /* Header: level[tag]: */
    apep_color_begin(out, &caps, role);
    fputs(apep_level_name(lvl), out);
    apep_color_end(out, &caps);

    if (tag && tag[0])
    {
        fputc('[', out);
        apep_color_begin(out, &caps, APEP_CR_LABEL);
        fputs(tag, out);
        apep_color_end(out, &caps);
        fputc(']', out);
    }

    fputs(": ", out);
    fputs(message ? message : "", out);
    fputc('\n', out);
}
