#include "../include/apep/apep.h"
#include "apep_internal.h"

#include <ctype.h>
#include <string.h>

static void apep_print_notes(FILE *out, const apep_note_t *notes, size_t notes_count)
{
    for (size_t i = 0; i < notes_count; i++)
    {
        const char *k = (notes[i].kind && notes[i].kind[0]) ? notes[i].kind : "note";
        const char *m = notes[i].message ? notes[i].message : "";
        fprintf(out, "  = %s: %s\n", k, m);
    }
}

static int apep_should_show_ascii(int width)
{
    /* Conservative: ASCII column needs some space */
    return (width >= 90) ? 1 : 0;
}

static void apep_print_hex_line(
    FILE *out,
    const apep_caps_t *caps,
    const uint8_t *data,
    size_t data_size,
    size_t line_off,
    int bpl,
    apep_span_t span,
    int show_ascii)
{
    /* Offset */
    fprintf(out, "%08lx: ", (unsigned long)line_off);

    /* Hex bytes (with span highlighting using colors or markers) */
    for (int i = 0; i < bpl; i++)
    {
        size_t idx = line_off + (size_t)i;

        int in_range = 0;
        if (span.length > 0)
        {
            size_t s0 = span.offset;
            size_t s1 = span.offset + span.length; /* end-exclusive */
            if (idx >= s0 && idx < s1)
                in_range = 1;
        }

        /* add extra space between two blocks */
        if (i == 8)
            fputc(' ', out);

        if (idx < data_size)
        {
            if (in_range)
            {
                /* Use color highlighting if available, otherwise use markers */
                if (caps && caps->color)
                {
                    apep_color_begin(out, caps, APEP_CR_HIGHLIGHT);
                    fprintf(out, "%02X", (unsigned)data[idx]);
                    apep_color_end(out, caps);
                    fputc(' ', out);
                }
                else
                {
                    /* Fallback to markers for non-color terminals */
                    fputc('*', out);
                    fprintf(out, "%02X", (unsigned)data[idx]);
                }
            }
            else
            {
                fprintf(out, "%02X ", (unsigned)data[idx]);
            }
        }
        else
        {
            /* Past end: keep columns aligned */
            fputs("   ", out);
        }
    }

    if (!show_ascii)
    {
        fputc('\n', out);
        return;
    }

    /* ASCII preview */
    fputs(" |", out);
    for (int i = 0; i < bpl; i++)
    {
        size_t idx = line_off + (size_t)i;

        int in_range = 0;
        if (span.length > 0)
        {
            size_t s0 = span.offset;
            size_t s1 = span.offset + span.length;
            if (idx >= s0 && idx < s1)
                in_range = 1;
        }

        char c = ' ';
        if (idx < data_size)
        {
            unsigned ch = data[idx];
            c = (isprint((int)ch) ? (char)ch : '.');
        }

        /* Highlight ASCII characters in range too */
        if (in_range && caps && caps->color)
        {
            apep_color_begin(out, caps, APEP_CR_HIGHLIGHT);
            fputc(c, out);
            apep_color_end(out, caps);
        }
        else
        {
            fputc(c, out);
        }
    }
    fputs("|\n", out);
}

void apep_print_hex_diagnostic(
    const apep_options_t *opt_in,
    apep_severity_t sev,
    const char *code,
    const char *message,
    const char *blob_name,
    const uint8_t *data,
    size_t data_size,
    apep_span_t span,
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

    apep_caps_t caps = apep_detect_caps(out, opt);

    const char *arrow = caps.unicode ? "→" : "->";

    /* Header */
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

    apep_color_begin(out, &caps, APEP_CR_DIM);
    fprintf(out, "  %s %s:+0x%lx (span %lu bytes)\n",
            arrow,
            (blob_name && blob_name[0]) ? blob_name : "<blob>",
            (unsigned long)span.offset,
            (unsigned long)span.length);
    apep_color_end(out, &caps);

    if (!data || data_size == 0)
    {
        fprintf(out, "  (no binary data available)\n");
        apep_print_notes(out, notes, notes_count);
        return;
    }

    /* Determine bytes per line and context window */
    int bpl = opt->hex_bytes_per_line;
    if (bpl <= 0)
        bpl = 16;
    if (bpl != 8 && bpl != 16 && bpl != 32)
        bpl = 16;

    int ctx = opt->hex_context_bytes;
    if (ctx <= 0)
        ctx = 64;
    if (ctx > 4096)
        ctx = 4096;

    /* Clamp span.offset into [0, data_size] safely */
    if (span.offset > data_size)
        span.offset = data_size;

    /* Compute window [win_start, win_end) */
    size_t win_start = 0;
    size_t win_end = data_size;

    if (data_size > (size_t)ctx)
    {
        size_t half = (size_t)ctx / 2;
        if (span.offset > half)
            win_start = span.offset - half;
        else
            win_start = 0;

        win_end = win_start + (size_t)ctx;
        if (win_end > data_size)
        {
            win_end = data_size;
            if (win_end > (size_t)ctx)
                win_start = win_end - (size_t)ctx;
            else
                win_start = 0;
        }
    }

    /* Align window start to line boundary */
    win_start = (win_start / (size_t)bpl) * (size_t)bpl;
    if (win_start > data_size)
        win_start = data_size;

    /* Align window end to line boundary (round up) */
    size_t aligned_end = ((win_end + (size_t)bpl - 1) / (size_t)bpl) * (size_t)bpl;
    if (aligned_end > data_size)
        aligned_end = data_size;
    win_end = aligned_end;

    int show_ascii = apep_should_show_ascii(caps.width);

    fprintf(out, "  (binary size: %lu bytes, window: 0x%lx..0x%lx)\n",
            (unsigned long)data_size, (unsigned long)win_start, (unsigned long)win_end);

    /* Print hexdump lines */
    for (size_t off = win_start; off < win_end; off += (size_t)bpl)
    {
        apep_print_hex_line(out, &caps, data, data_size, off, bpl, span, show_ascii);
    }

    apep_print_notes(out, notes, notes_count);
}
