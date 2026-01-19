#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include "apep_internal.h"
#include <stdio.h>
#include <string.h>

void apep_print_text_diagnostic_with_suggestion(
    const apep_options_t *opt_in,
    apep_severity_t sev,
    const char *code,
    const char *message,
    const apep_text_source_t *src,
    apep_loc_t loc,
    int span_len_cols,
    const apep_note_t *notes,
    size_t notes_count,
    const apep_suggestion_t *suggestion)
{
    /* First print the normal diagnostic */
    apep_print_text_diagnostic(opt_in, sev, code, message, src, loc, span_len_cols, notes, notes_count);

    /* Then add the suggestion */
    if (!suggestion)
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

    fprintf(out, "\n  ");
    apep_color_begin(out, &caps, APEP_CR_LABEL);
    fputs(suggestion->label ? suggestion->label : "help", out);
    apep_color_end(out, &caps);
    fputs(": ", out);

    if (suggestion->code)
    {
        fputs("try this instead:\n", out);

        /* Get the line from source */
        const char *line_ptr;
        size_t line_len;
        if (src->get_line(src->user, suggestion->loc.line, &line_ptr, &line_len))
        {
            /* Show original with X mark */
            fprintf(out, "      | ");
            fwrite(line_ptr, 1, line_len, out);
            fputc('\n', out);

            /* Show suggestion with checkmark */
            fprintf(out, "      | ");
            apep_color_begin(out, &caps, APEP_CR_LVL_INFO);
            fputs(suggestion->code, out);
            apep_color_end(out, &caps);
            fputc('\n', out);
        }
        else
        {
            fprintf(out, "      | ");
            apep_color_begin(out, &caps, APEP_CR_LVL_INFO);
            fputs(suggestion->code, out);
            apep_color_end(out, &caps);
            fputc('\n', out);
        }
    }
    fputc('\n', out);
}
