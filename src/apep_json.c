#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include "apep_internal.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define isatty _isatty
/* fileno is already defined in MinGW's stdio.h */
#ifndef fileno
#define fileno _fileno
#endif
/* Define this constant if not available in older Windows headers */
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <unistd.h>
#endif

/* Color codes for JSON syntax highlighting */
#define JSON_KEY_COLOR "\x1b[36m"       /* cyan for keys */
#define JSON_STRING_COLOR "\x1b[32m"    /* green for strings */
#define JSON_NUMBER_COLOR "\x1b[33m"    /* yellow for numbers */
#define JSON_BRACKET_COLOR "\x1b[1;37m" /* bold white for brackets */
#define JSON_RESET "\x1b[0m"

/* Escape JSON string */
static void json_escape_string(FILE *out, const char *str, int use_colors)
{
    if (!str)
    {
        if (use_colors)
            fputs(JSON_NUMBER_COLOR, out);
        fputs("null", out);
        if (use_colors)
            fputs(JSON_RESET, out);
        return;
    }

    if (use_colors)
        fputs(JSON_STRING_COLOR, out);

    fputc('"', out);
    for (const char *p = str; *p; p++)
    {
        switch (*p)
        {
        case '"':
            fputs("\\\"", out);
            break;
        case '\\':
            fputs("\\\\", out);
            break;
        case '\n':
            fputs("\\n", out);
            break;
        case '\r':
            fputs("\\r", out);
            break;
        case '\t':
            fputs("\\t", out);
            break;
        default:
            if (*p < 32)
            {
                fprintf(out, "\\u%04x", (unsigned char)*p);
            }
            else
            {
                fputc(*p, out);
            }
            break;
        }
    }
    fputc('"', out);

    if (use_colors)
        fputs(JSON_RESET, out);
}

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
    size_t notes_count)
{
    if (!out)
        out = stderr;

    /* Detect if we should use colors (TTY check) */
    int use_colors = isatty(fileno(out));

#ifdef _WIN32
    /* Windows terminal color support */
    if (use_colors)
    {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        if (GetConsoleMode(hOut, &mode))
        {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
    }
#endif

    /* Opening brace */
    if (use_colors)
        fputs(JSON_BRACKET_COLOR, out);
    fputs("{\n", out);
    if (use_colors)
        fputs(JSON_RESET, out);

    /* Severity */
    fputs("  ", out);
    if (use_colors)
        fputs(JSON_KEY_COLOR, out);
    fputs("\"severity\"", out);
    if (use_colors)
        fputs(JSON_RESET, out);
    fputs(": ", out);
    json_escape_string(out, apep_severity_name(sev), use_colors);
    fputs(",\n", out);

    /* Code */
    fputs("  ", out);
    if (use_colors)
        fputs(JSON_KEY_COLOR, out);
    fputs("\"code\"", out);
    if (use_colors)
        fputs(JSON_RESET, out);
    fputs(": ", out);
    json_escape_string(out, code, use_colors);
    fputs(",\n", out);

    /* Message */
    fputs("  ", out);
    if (use_colors)
        fputs(JSON_KEY_COLOR, out);
    fputs("\"message\"", out);
    if (use_colors)
        fputs(JSON_RESET, out);
    fputs(": ", out);
    json_escape_string(out, message, use_colors);
    fputs(",\n", out);

    /* Location */
    fputs("  ", out);
    if (use_colors)
        fputs(JSON_KEY_COLOR, out);
    fputs("\"location\"", out);
    if (use_colors)
        fputs(JSON_RESET, out);
    fputs(": ", out);
    if (use_colors)
        fputs(JSON_BRACKET_COLOR, out);
    fputs("{\n", out);
    if (use_colors)
        fputs(JSON_RESET, out);

    fputs("    ", out);
    if (use_colors)
        fputs(JSON_KEY_COLOR, out);
    fputs("\"file\"", out);
    if (use_colors)
        fputs(JSON_RESET, out);
    fputs(": ", out);
    json_escape_string(out, file, use_colors);
    fputs(",\n", out);

    fputs("    ", out);
    if (use_colors)
        fputs(JSON_KEY_COLOR, out);
    fputs("\"line\"", out);
    if (use_colors)
        fputs(JSON_RESET, out);
    fputs(": ", out);
    if (use_colors)
        fputs(JSON_NUMBER_COLOR, out);
    fprintf(out, "%d", line);
    if (use_colors)
        fputs(JSON_RESET, out);
    fputs(",\n", out);

    fputs("    ", out);
    if (use_colors)
        fputs(JSON_KEY_COLOR, out);
    fputs("\"column\"", out);
    if (use_colors)
        fputs(JSON_RESET, out);
    fputs(": ", out);
    if (use_colors)
        fputs(JSON_NUMBER_COLOR, out);
    fprintf(out, "%d", col);
    if (use_colors)
        fputs(JSON_RESET, out);
    fputs(",\n", out);

    fputs("    ", out);
    if (use_colors)
        fputs(JSON_KEY_COLOR, out);
    fputs("\"span_length\"", out);
    if (use_colors)
        fputs(JSON_RESET, out);
    fputs(": ", out);
    if (use_colors)
        fputs(JSON_NUMBER_COLOR, out);
    fprintf(out, "%d", span_len);
    if (use_colors)
        fputs(JSON_RESET, out);
    fputs("\n  ", out);

    if (use_colors)
        fputs(JSON_BRACKET_COLOR, out);
    fputs("}", out);
    if (use_colors)
        fputs(JSON_RESET, out);

    /* Notes */
    if (notes_count > 0)
    {
        fputs(",\n  ", out);
        if (use_colors)
            fputs(JSON_KEY_COLOR, out);
        fputs("\"notes\"", out);
        if (use_colors)
            fputs(JSON_RESET, out);
        fputs(": ", out);
        if (use_colors)
            fputs(JSON_BRACKET_COLOR, out);
        fputs("[\n", out);
        if (use_colors)
            fputs(JSON_RESET, out);

        for (size_t i = 0; i < notes_count; i++)
        {
            fputs("    ", out);
            if (use_colors)
                fputs(JSON_BRACKET_COLOR, out);
            fputs("{\n", out);
            if (use_colors)
                fputs(JSON_RESET, out);

            fputs("      ", out);
            if (use_colors)
                fputs(JSON_KEY_COLOR, out);
            fputs("\"kind\"", out);
            if (use_colors)
                fputs(JSON_RESET, out);
            fputs(": ", out);
            json_escape_string(out, notes[i].kind, use_colors);
            fputs(",\n", out);

            fputs("      ", out);
            if (use_colors)
                fputs(JSON_KEY_COLOR, out);
            fputs("\"message\"", out);
            if (use_colors)
                fputs(JSON_RESET, out);
            fputs(": ", out);
            json_escape_string(out, notes[i].message, use_colors);
            fputs("\n    ", out);

            if (use_colors)
                fputs(JSON_BRACKET_COLOR, out);
            fputs("}", out);
            if (use_colors)
                fputs(JSON_RESET, out);

            if (i + 1 < notes_count)
                fputs(",", out);
            fputs("\n", out);
        }

        fputs("  ", out);
        if (use_colors)
            fputs(JSON_BRACKET_COLOR, out);
        fputs("]", out);
        if (use_colors)
            fputs(JSON_RESET, out);
    }

    fputs("\n", out);
    if (use_colors)
        fputs(JSON_BRACKET_COLOR, out);
    fputs("}\n", out);
    if (use_colors)
        fputs(JSON_RESET, out);
}
