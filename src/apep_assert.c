#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include "apep_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void apep_assert_failed(
    const char *expr,
    const char *msg,
    const char *file,
    int line,
    const char *func)
{
    apep_options_t opt;
    apep_options_default(&opt);
    apep_caps_t caps = apep_detect_caps(stderr, &opt);

    fprintf(stderr, "\n");
    apep_color_begin(stderr, &caps, APEP_CR_SEV_ERROR);
    fprintf(stderr, "Assertion failed");
    apep_color_end(stderr, &caps);
    fprintf(stderr, ": %s\n", expr);

    fprintf(stderr, "  -> %s:%d in %s()\n", file, line, func);

    if (msg && msg[0])
    {
        fprintf(stderr, "  = ");
        apep_color_begin(stderr, &caps, APEP_CR_LABEL);
        fprintf(stderr, "message");
        apep_color_end(stderr, &caps);
        fprintf(stderr, ": %s\n", msg);
    }

    /* Print stack trace if available */
    fprintf(stderr, "\n");
    apep_stack_print(&opt);
    fprintf(stderr, "\n");
}

void apep_assert_failed_fmt(
    const char *expr,
    const char *file,
    int line,
    const char *func,
    const char *fmt,
    ...)
{
    apep_options_t opt;
    apep_options_default(&opt);
    apep_caps_t caps = apep_detect_caps(stderr, &opt);

    fprintf(stderr, "\n");
    apep_color_begin(stderr, &caps, APEP_CR_SEV_ERROR);
    fprintf(stderr, "Assertion failed");
    apep_color_end(stderr, &caps);
    fprintf(stderr, ": %s\n", expr);

    fprintf(stderr, "  -> %s:%d in %s()\n", file, line, func);

    if (fmt && fmt[0])
    {
        fprintf(stderr, "  = ");
        apep_color_begin(stderr, &caps, APEP_CR_LABEL);
        fprintf(stderr, "message");
        apep_color_end(stderr, &caps);
        fprintf(stderr, ": ");

        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);

        fprintf(stderr, "\n");
    }

    /* Print stack trace if available */
    fprintf(stderr, "\n");
    apep_stack_print(&opt);
    fprintf(stderr, "\n");
}
