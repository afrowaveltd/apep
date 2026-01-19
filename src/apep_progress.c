#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include "apep_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct apep_progress
{
    char *label;
    size_t total;
    size_t current;
    FILE *out;
    int width;
    int is_tty;
};

apep_progress_t *apep_progress_start(
    const apep_options_t *opt,
    const char *label,
    size_t total)
{
    apep_progress_t *prog = malloc(sizeof(apep_progress_t));
    if (!prog)
        return NULL;

    prog->label = label ? strdup(label) : NULL;
    prog->total = total;
    prog->current = 0;
    prog->out = (opt && opt->out) ? opt->out : stderr;

    apep_caps_t caps = apep_detect_caps(prog->out, opt);
    prog->is_tty = caps.is_tty;
    prog->width = caps.width > 80 ? 50 : 30;

    return prog;
}

void apep_progress_update(apep_progress_t *prog, size_t current)
{
    if (!prog)
        return;

    prog->current = current;

    if (!prog->is_tty)
    {
        /* Non-TTY: just print occasional updates */
        if (current % (prog->total / 10 + 1) == 0 || current == prog->total)
        {
            fprintf(prog->out, "[%s] %zu/%zu (%.0f%%)\n",
                    prog->label ? prog->label : "Progress",
                    current, prog->total,
                    100.0 * current / prog->total);
        }
        return;
    }

    /* TTY: animated progress bar */
    fprintf(prog->out, "\r[%s] ", prog->label ? prog->label : "Progress");

    int filled = (int)((double)current / prog->total * prog->width);
    fputc('[', prog->out);
    for (int i = 0; i < prog->width; i++)
    {
        if (i < filled)
            fputc('=', prog->out);
        else if (i == filled)
            fputc('>', prog->out);
        else
            fputc(' ', prog->out);
    }
    fputc(']', prog->out);

    fprintf(prog->out, " %zu/%zu (%.0f%%)", current, prog->total,
            100.0 * current / prog->total);

    fflush(prog->out);
}

void apep_progress_done(apep_progress_t *prog)
{
    if (!prog)
        return;

    apep_progress_update(prog, prog->total);

    if (prog->is_tty)
        fputc('\n', prog->out);

    free(prog->label);
    free(prog);
}
