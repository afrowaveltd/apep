#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include "apep_internal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void apep_print_text_diagnostic_multi(
    const apep_options_t *opt_in,
    apep_severity_t sev,
    const char *code,
    const char *message,
    const apep_text_source_t *src,
    const apep_text_span_t *spans,
    size_t spans_count,
    const apep_note_t *notes,
    size_t notes_count)
{
    if (!src || !spans || spans_count == 0)
        return;

    apep_options_t def;
    const apep_options_t *opt = opt_in;
    if (!opt)
    {
        apep_options_default(&def);
        opt = &def;
    }
    FILE *out = opt->out ? opt->out : stderr;
    apep_caps_t caps = apep_detect_caps(out, opt);

    /* Print header */
    apep_color_begin(out, &caps, APEP_CR_SEV_ERROR + sev);
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
    if (message)
        fputs(message, out);
    fputc('\n', out);

    /* Print location (use first span) */
    fprintf(out, "  -> %s:%d:%d\n",
            src->name ? src->name : "<input>",
            spans[0].loc.line,
            spans[0].loc.col);

    /* Get all affected lines */
    int min_line = spans[0].loc.line;
    int max_line = spans[0].loc.line;

    for (size_t i = 1; i < spans_count; i++)
    {
        if (spans[i].loc.line < min_line)
            min_line = spans[i].loc.line;
        if (spans[i].loc.line > max_line)
            max_line = spans[i].loc.line;
    }

    /* Print lines with multiple highlights */
    for (int line_no = min_line; line_no <= max_line; line_no++)
    {
        const char *line_ptr;
        size_t line_len;

        if (!src->get_line(src->user, line_no, &line_ptr, &line_len))
            continue;

        fprintf(out, "      |\n");
        fprintf(out, " %4d | ", line_no);
        fwrite(line_ptr, 1, line_len, out);
        fputc('\n', out);

        /* Print carets for all spans on this line */
        for (size_t i = 0; i < spans_count; i++)
        {
            if (spans[i].loc.line != line_no)
                continue;

            fprintf(out, "      | ");

            /* Spaces before caret */
            for (int j = 1; j < spans[i].loc.col; j++)
                fputc(' ', out);

            /* Caret(s) */
            apep_color_begin(out, &caps, APEP_CR_CARET);
            for (int j = 0; j < spans[i].length; j++)
                fputc('^', out);
            apep_color_end(out, &caps);

            /* Label if present */
            if (spans[i].label)
            {
                fputc(' ', out);
                apep_color_begin(out, &caps, APEP_CR_DIM);
                fputs(spans[i].label, out);
                apep_color_end(out, &caps);
            }
            fputc('\n', out);
        }
    }

    /* Print notes */
    if (notes && notes_count > 0)
    {
        fprintf(out, "      |\n");
        for (size_t i = 0; i < notes_count; i++)
        {
            fputs("  = ", out);
            apep_color_begin(out, &caps, APEP_CR_LABEL);
            fputs(notes[i].kind ? notes[i].kind : "note", out);
            apep_color_end(out, &caps);
            fputs(": ", out);
            if (notes[i].message)
                fputs(notes[i].message, out);
            fputc('\n', out);
        }
    }

    fputc('\n', out);
}
