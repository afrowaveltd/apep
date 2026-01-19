#include "../include/apep/apep.h"

#include <stdlib.h>
#include <string.h>

const char *apep_severity_name(apep_severity_t sev)
{
    switch (sev)
    {
    case APEP_SEV_ERROR:
        return "error";
    case APEP_SEV_WARN:
        return "warning";
    case APEP_SEV_NOTE:
        return "note";
    default:
        return "note";
    }
}

void apep_options_default(apep_options_t *opt)
{
    if (!opt)
        return;
    opt->out = stderr;

    opt->style = APEP_STYLE_FULL;

    opt->color = APEP_COLOR_AUTO;
    opt->unicode = APEP_UNICODE_AUTO;

    opt->width_override = 0;

    opt->context_lines = 2;

    opt->hex_bytes_per_line = 16;
    opt->hex_context_bytes = 64;

    /* hard overrides (default: off) */
    opt->force_no_color = 0;
    opt->force_ascii = 0;
}

/* ----------------------------
Text source from string
---------------------------- */

static int apep_get_line_from_string(
    void *user,
    int line_no_1based,
    const char **line_ptr,
    size_t *line_len)
{
    if (!user || !line_ptr || !line_len)
        return 0;
    if (line_no_1based <= 0)
        return 0;

    const char *text = (const char *)user;

    int current = 1;
    const char *p = text;
    const char *line_start = p;

    while (*p && current < line_no_1based)
    {
        if (*p == '\n')
        {
            current++;
            line_start = p + 1;
        }
        p++;
    }

    if (current != line_no_1based)
    {
        return 0;
    }

    /* If we reached the end of string and line_start points to null terminator,
       this line doesn't exist (e.g., asking for line 2 when input is "line1\n") */
    if (*line_start == '\0')
    {
        return 0;
    }

    /* find end of line (exclude '\n' and optional '\r') */
    const char *end = line_start;
    while (*end && *end != '\n')
        end++;

    /* trim CR if present */
    const char *trimmed_end = end;
    if (trimmed_end > line_start && trimmed_end[-1] == '\r')
        trimmed_end--;

    *line_ptr = line_start;
    *line_len = (size_t)(trimmed_end - line_start);
    return 1;
}

apep_text_source_t apep_text_source_from_string(const char *name, const char *text)
{
    apep_text_source_t src;
    src.name = name ? name : "<input>";
    src.get_line = apep_get_line_from_string;
    src.user = (void *)(text ? text : "");
    return src;
}

const char *apep_level_name(apep_level_t lvl)
{
    switch (lvl)
    {
    case APEP_LVL_TRACE:
        return "trace";
    case APEP_LVL_DEBUG:
        return "debug";
    case APEP_LVL_INFO:
        return "info";
    case APEP_LVL_WARN:
        return "warn";
    case APEP_LVL_ERROR:
        return "error";
    case APEP_LVL_CRITICAL:
        return "critical";
    default:
        return "info";
    }
}