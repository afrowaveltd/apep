#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include <stdio.h>
#include <string.h>

static apep_stack_frame_t stack[APEP_MAX_STACK_DEPTH];
static int stack_depth = 0;

void apep_stack_push(const char *func, const char *file, int line)
{
    if (stack_depth >= APEP_MAX_STACK_DEPTH)
        return;

    stack[stack_depth].function = func;
    stack[stack_depth].file = file;
    stack[stack_depth].line = line;
    stack_depth++;
}

void apep_stack_pop(void)
{
    if (stack_depth > 0)
        stack_depth--;
}

void apep_stack_print(const apep_options_t *opt)
{
    FILE *out = opt && opt->out ? opt->out : stderr;

    if (stack_depth == 0)
    {
        fprintf(out, "Stack trace: (empty)\n");
        return;
    }

    fprintf(out, "Stack trace:\n");
    for (int i = stack_depth - 1; i >= 0; i--)
    {
        fprintf(out, "  #%d: %s() at %s:%d\n",
                stack_depth - 1 - i,
                stack[i].function ? stack[i].function : "???",
                stack[i].file ? stack[i].file : "???",
                stack[i].line);
    }
}

void apep_stack_clear(void)
{
    stack_depth = 0;
}
