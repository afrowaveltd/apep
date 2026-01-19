#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include "apep_internal.h"
#include <string.h>

static apep_color_scheme_t current_scheme = APEP_SCHEME_DEFAULT;
static apep_custom_colors_t custom_colors = {0};

/* Color scheme definitions */
static const struct
{
    const char *error;
    const char *warning;
    const char *note;
    const char *highlight;
    const char *caret;
    const char *label;
    const char *dim;
} color_schemes[] = {
    [APEP_SCHEME_DEFAULT] = {
        .error = "\x1b[1;31m",        /* bold red */
        .warning = "\x1b[33m",        /* yellow */
        .note = "\x1b[34m",           /* blue */
        .highlight = "\x1b[1;33;41m", /* bold yellow on red */
        .caret = "\x1b[1;31m",        /* bold red */
        .label = "\x1b[1m",           /* bold */
        .dim = "\x1b[2m"              /* dim */
    },
    [APEP_SCHEME_DARK] = {
        .error = "\x1b[1;91m", /* bold bright red */
        .warning = "\x1b[93m", /* bright yellow */
        .note = "\x1b[96m",    /* bright cyan */
        .highlight = "\x1b[1;93;41m",
        .caret = "\x1b[1;91m",
        .label = "\x1b[1;97m",  /* bold bright white */
        .dim = "\x1b[38;5;240m" /* gray */
    },
    [APEP_SCHEME_LIGHT] = {
        .error = "\x1b[31m",         /* red (not bold) */
        .warning = "\x1b[38;5;130m", /* orange */
        .note = "\x1b[34m",          /* blue */
        .highlight = "\x1b[33;41m",
        .caret = "\x1b[31m",
        .label = "\x1b[1;30m", /* bold black */
        .dim = "\x1b[90m"      /* bright black */
    },
    [APEP_SCHEME_COLORBLIND] = {.error = "\x1b[1;35m",        /* bold magenta */
                                .warning = "\x1b[36m",        /* cyan */
                                .note = "\x1b[34m",           /* blue */
                                .highlight = "\x1b[1;36;45m", /* cyan on magenta */
                                .caret = "\x1b[1;35m",
                                .label = "\x1b[1m",
                                .dim = "\x1b[2m"}};

void apep_set_color_scheme(apep_color_scheme_t scheme)
{
    if (scheme >= APEP_SCHEME_DEFAULT && scheme <= APEP_SCHEME_CUSTOM)
    {
        current_scheme = scheme;
    }
}

void apep_set_custom_colors(const apep_custom_colors_t *colors)
{
    if (colors)
    {
        custom_colors = *colors;
        current_scheme = APEP_SCHEME_CUSTOM;
    }
}

apep_color_scheme_t apep_get_color_scheme(void)
{
    return current_scheme;
}

/* Get color code for role based on current scheme */
const char *apep_get_color_for_role(apep_color_role_t role)
{
    const apep_custom_colors_t *colors;

    if (current_scheme == APEP_SCHEME_CUSTOM)
    {
        colors = &custom_colors;
    }
    else
    {
        colors = (const apep_custom_colors_t *)&color_schemes[current_scheme];
    }

    switch (role)
    {
    case APEP_CR_SEV_ERROR:
    case APEP_CR_LVL_ERROR:
    case APEP_CR_LVL_CRITICAL:
        return colors->error;
    case APEP_CR_SEV_WARN:
    case APEP_CR_LVL_WARN:
        return colors->warning;
    case APEP_CR_SEV_NOTE:
    case APEP_CR_LVL_INFO:
        return colors->note;
    case APEP_CR_HIGHLIGHT:
        return colors->highlight;
    case APEP_CR_CARET:
        return colors->caret;
    case APEP_CR_LABEL:
        return colors->label;
    case APEP_CR_DIM:
    case APEP_CR_LVL_TRACE:
    case APEP_CR_LVL_DEBUG:
        return colors->dim;
    default:
        return "\x1b[0m";
    }
}
