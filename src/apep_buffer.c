#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFERED_DIAGS 1024

typedef struct buffered_diag
{
    apep_severity_t sev;
    char *code;
    char *message;
    char *file;
    int line;
    int col;
} buffered_diag_t;

struct apep_diagnostic_buffer
{
    buffered_diag_t *diags;
    size_t count;
    size_t capacity;
};

apep_diagnostic_buffer_t *apep_buffer_create(void)
{
    apep_diagnostic_buffer_t *buf = malloc(sizeof(apep_diagnostic_buffer_t));
    if (!buf)
        return NULL;

    buf->capacity = 16;
    buf->count = 0;
    buf->diags = malloc(sizeof(buffered_diag_t) * buf->capacity);

    if (!buf->diags)
    {
        free(buf);
        return NULL;
    }

    return buf;
}

static char *str_dup(const char *s)
{
    if (!s)
        return NULL;
    size_t len = strlen(s);
    char *copy = malloc(len + 1);
    if (copy)
        memcpy(copy, s, len + 1);
    return copy;
}

void apep_buffer_add(
    apep_diagnostic_buffer_t *buf,
    apep_severity_t sev,
    const char *code,
    const char *message,
    const char *file,
    int line,
    int col)
{
    if (!buf || buf->count >= MAX_BUFFERED_DIAGS)
        return;

    /* Expand if needed */
    if (buf->count >= buf->capacity)
    {
        size_t new_cap = buf->capacity * 2;
        buffered_diag_t *new_diags = realloc(buf->diags, sizeof(buffered_diag_t) * new_cap);
        if (!new_diags)
            return;
        buf->diags = new_diags;
        buf->capacity = new_cap;
    }

    buffered_diag_t *d = &buf->diags[buf->count++];
    d->sev = sev;
    d->code = str_dup(code);
    d->message = str_dup(message);
    d->file = str_dup(file);
    d->line = line;
    d->col = col;
}

static int compare_diags(const void *a, const void *b)
{
    const buffered_diag_t *da = a;
    const buffered_diag_t *db = b;

    /* Sort by file first */
    if (da->file && db->file)
    {
        int cmp = strcmp(da->file, db->file);
        if (cmp != 0)
            return cmp;
    }

    /* Then by line */
    if (da->line != db->line)
        return da->line - db->line;

    /* Then by column */
    return da->col - db->col;
}

void apep_buffer_flush(
    apep_diagnostic_buffer_t *buf,
    const apep_options_t *opt,
    int sort_by_location)
{
    if (!buf)
        return;

    if (sort_by_location && buf->count > 1)
    {
        qsort(buf->diags, buf->count, sizeof(buffered_diag_t), compare_diags);
    }

    for (size_t i = 0; i < buf->count; i++)
    {
        buffered_diag_t *d = &buf->diags[i];
        apep_print_json_diagnostic(
            opt ? opt->out : stderr,
            d->sev,
            d->code,
            d->message,
            d->file,
            d->line,
            d->col,
            1,
            NULL,
            0);
    }

    apep_buffer_clear(buf);
}

void apep_buffer_clear(apep_diagnostic_buffer_t *buf)
{
    if (!buf)
        return;

    for (size_t i = 0; i < buf->count; i++)
    {
        free(buf->diags[i].code);
        free(buf->diags[i].message);
        free(buf->diags[i].file);
    }
    buf->count = 0;
}

void apep_buffer_destroy(apep_diagnostic_buffer_t *buf)
{
    if (!buf)
        return;

    apep_buffer_clear(buf);
    free(buf->diags);
    free(buf);
}

size_t apep_buffer_count(const apep_diagnostic_buffer_t *buf)
{
    return buf ? buf->count : 0;
}
