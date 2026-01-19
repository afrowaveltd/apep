#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include "apep_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

struct apep_perf_timer
{
    char *label;
    double start_time;
};

static double get_time_ms(void)
{
#ifdef _WIN32
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart * 1000.0 / (double)frequency.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
#endif
}

apep_perf_timer_t *apep_perf_start(const char *label)
{
    apep_perf_timer_t *timer = malloc(sizeof(apep_perf_timer_t));
    if (!timer)
        return NULL;

    timer->label = label ? strdup(label) : NULL;
    timer->start_time = get_time_ms();
    return timer;
}

void apep_perf_end(apep_perf_timer_t *timer, const apep_options_t *opt)
{
    if (!timer)
        return;

    double end_time = get_time_ms();
    double elapsed = end_time - timer->start_time;

    FILE *out = (opt && opt->out) ? opt->out : stderr;

    fprintf(out, "[PERF] %s: %.3fms\n",
            timer->label ? timer->label : "unnamed",
            elapsed);

    free(timer->label);
    free(timer);
}
