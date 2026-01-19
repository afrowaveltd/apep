#ifndef APEP_INTERNAL_H
#define APEP_INTERNAL_H

#include "../include/apep/apep.h"

typedef enum apep_color_role
{
    APEP_CR_RESET = 0,
    APEP_CR_SEV_ERROR,
    APEP_CR_SEV_WARN,
    APEP_CR_SEV_NOTE,
    APEP_CR_LABEL,
    APEP_CR_DIM,

    /* Log-level roles (for future apep_print_message / logger integration) */
    APEP_CR_LVL_TRACE,
    APEP_CR_LVL_DEBUG,
    APEP_CR_LVL_INFO,
    APEP_CR_LVL_WARN,
    APEP_CR_LVL_ERROR,
    APEP_CR_LVL_CRITICAL,

    /* Highlighting roles */
    APEP_CR_HIGHLIGHT, /* For highlighted spans in hex/text */
    APEP_CR_CARET      /* For caret/pointer symbols */
} apep_color_role_t;

/* Begin/end a colored segment. If caps->color == 0, these do nothing. */
void apep_color_begin(FILE *out, const apep_caps_t *caps, apep_color_role_t role);
void apep_color_end(FILE *out, const apep_caps_t *caps);

/* Get color code for role based on current color scheme */
const char *apep_get_color_for_role(apep_color_role_t role);

#endif
