#include "apep_internal.h"

static void apep_write(FILE *out, const char *s)
{
    if (!out || !s)
        return;
    fputs(s, out);
}

void apep_color_begin(FILE *out, const apep_caps_t *caps, apep_color_role_t role)
{
    if (!out || !caps || !caps->color)
        return;

    /* ANSI SGR sequences. Keep it minimal and readable. */
    switch (role)
    {
    case APEP_CR_SEV_ERROR:
        apep_write(out, "\x1b[1;31m");
        break; /* bold red */
    case APEP_CR_SEV_WARN:
        apep_write(out, "\x1b[33m");
        break; /* yellow */
    case APEP_CR_SEV_NOTE:
        apep_write(out, "\x1b[34m");
        break; /* blue */
    case APEP_CR_LABEL:
        apep_write(out, "\x1b[1m");
        break; /* bold */
    case APEP_CR_DIM:
        apep_write(out, "\x1b[2m");
        break; /* dim */

    /* Log levels: conservative palette */
    case APEP_CR_LVL_TRACE:
        apep_write(out, "\x1b[2m");
        break; /* dim */
    case APEP_CR_LVL_DEBUG:
        apep_write(out, "\x1b[36m");
        break; /* cyan */
    case APEP_CR_LVL_INFO:
        apep_write(out, "\x1b[32m");
        break; /* green */
    case APEP_CR_LVL_WARN:
        apep_write(out, "\x1b[33m");
        break; /* yellow */
    case APEP_CR_LVL_ERROR:
        apep_write(out, "\x1b[31m");
        break; /* red */
    case APEP_CR_LVL_CRITICAL:
        apep_write(out, "\x1b[1;31m");
        break; /* bold red */

    /* Highlighting */
    case APEP_CR_HIGHLIGHT:
        apep_write(out, "\x1b[1;33;41m");
        break; /* bold yellow on red background */
    case APEP_CR_CARET:
        apep_write(out, "\x1b[1;31m");
        break; /* bold red for ^ */

    case APEP_CR_RESET:
    default:
        apep_write(out, "\x1b[0m");
        break;
    }
}

void apep_color_end(FILE *out, const apep_caps_t *caps)
{
    if (!out || !caps || !caps->color)
        return;
    apep_write(out, "\x1b[0m");
}
