/**
 * @file apep_exception.c
 * @brief Exception handling implementation
 */

#include "apep/apep_exception.h"
#include "apep/apep.h"
#include "apep/apep_i18n.h"
#include "apep_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
/* dbghelp.h not available in MinGW by default */
#if defined(_MSC_VER)
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#define HAS_DBGHELP 1
#endif
#else
#include <execinfo.h>
#endif

/**
 * @brief Stack trace structure (platform-specific)
 */
typedef struct
{
    void *frames[64];
    int frame_count;
    char **symbols;
} apep_stack_trace_t;

static void apep_fputs_utf8(FILE *out, const char *s)
{
    if (!s || !out)
        return;

#ifdef _WIN32
    HANDLE h = NULL;
    if (out == stdout)
        h = GetStdHandle(STD_OUTPUT_HANDLE);
    else if (out == stderr)
        h = GetStdHandle(STD_ERROR_HANDLE);

    if (h && h != INVALID_HANDLE_VALUE)
    {
        DWORD mode = 0;
        if (GetConsoleMode(h, &mode))
        {
            int wlen = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
            if (wlen > 0)
            {
                WCHAR *wbuf = (WCHAR *)malloc(sizeof(WCHAR) * (size_t)wlen);
                if (wbuf)
                {
                    MultiByteToWideChar(CP_UTF8, 0, s, -1, wbuf, wlen);
                    DWORD written = 0;
                    WriteConsoleW(h, wbuf, (DWORD)(wlen - 1), &written, NULL);
                    free(wbuf);
                    return;
                }
            }
        }
    }
#endif

    fputs(s, out);
}

apep_exception_t *apep_exception_create(const char *type, const char *format, ...)
{
    apep_exception_t *ex = (apep_exception_t *)calloc(1, sizeof(apep_exception_t));
    if (!ex)
        return NULL;

    ex->type = type;
    ex->owns_message = 1;

    // Format message
    va_list args;
    va_start(args, format);

    // Calculate required size
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);

    if (size < 0)
    {
        va_end(args);
        free(ex);
        return NULL;
    }

    ex->message = (char *)malloc(size + 1);
    if (!ex->message)
    {
        va_end(args);
        free(ex);
        return NULL;
    }

    vsnprintf(ex->message, size + 1, format, args);
    va_end(args);

    return ex;
}

void apep_exception_set_source(apep_exception_t *ex, const char *file, int line)
{
    if (!ex)
        return;
    ex->source_file = file;
    ex->source_line = line;
}

void apep_exception_set_inner(apep_exception_t *ex, apep_exception_t *inner)
{
    if (!ex)
        return;
    ex->inner = inner;
    ex->owns_inner = 1;
}

void apep_exception_set_code(apep_exception_t *ex, int code)
{
    if (!ex)
        return;
    ex->error_code = code;
}

void apep_exception_capture_stack(apep_exception_t *ex)
{
    if (!ex)
        return;

    apep_stack_trace_t *stack = (apep_stack_trace_t *)calloc(1, sizeof(apep_stack_trace_t));
    if (!stack)
        return;

#if defined(_WIN32) && !defined(__GNUC__)
    // Windows stack trace using CaptureStackBackTrace (MSVC only)
    stack->frame_count = CaptureStackBackTrace(1, 64, stack->frames, NULL);
#elif !defined(_WIN32)
    // Unix stack trace using backtrace
    stack->frame_count = backtrace(stack->frames, 64);
    stack->symbols = backtrace_symbols(stack->frames, stack->frame_count);
#else
    // MinGW: stack trace not available
    stack->frame_count = 0;
#endif

    ex->stack_trace = stack;
}

static void print_stack_trace(FILE *out, const apep_stack_trace_t *stack, int indent)
{
    if (!stack || stack->frame_count <= 0)
        return;

    char indent_str[128];
    memset(indent_str, ' ', indent);
    indent_str[indent] = '\0';

#ifdef HAS_DBGHELP
    // Windows: Use SymFromAddr for symbols (MSVC only)
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    SYMBOL_INFO *symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256, 1);
    if (symbol)
    {
        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        for (int i = 0; i < stack->frame_count; i++)
        {
            DWORD64 address = (DWORD64)(stack->frames[i]);

            if (SymFromAddr(process, address, 0, symbol))
            {
                fprintf(out, "%s  #%d: %s at 0x%llx\n",
                        indent_str, i, symbol->Name, (unsigned long long)symbol->Address);
            }
            else
            {
                fprintf(out, "%s  #%d: <unknown> at 0x%p\n",
                        indent_str, i, stack->frames[i]);
            }
        }
        free(symbol);
    }

    SymCleanup(process);
#elif defined(_WIN32)
    // MinGW: Simple address dump
    for (int i = 0; i < stack->frame_count; i++)
    {
        fprintf(out, "%s  #%d: 0x%p\n",
                indent_str, i, stack->frames[i]);
    }
#else
    // Unix: Use backtrace_symbols
    if (stack->symbols)
    {
        for (int i = 0; i < stack->frame_count; i++)
        {
            // Parse symbol: "binary(function+offset) [address]"
            const char *sym = stack->symbols[i];
            fprintf(out, "%s  #%d: %s\n", indent_str, i, sym);
        }
    }
#endif
}

void apep_exception_print(const apep_options_t *opt, const apep_exception_t *ex)
{
    if (!ex)
        return;

    FILE *out = stdout;

    // Check if colors are supported
    apep_options_t default_opt = {0};
    if (!opt)
    {
        apep_options_default(&default_opt);
        opt = &default_opt;
    }

    apep_caps_t caps = apep_detect_caps(out, opt);

    // Exception type and message
    const char *error_color = caps.color ? "\033[1;31m" : "";
    const char *reset = caps.color ? "\033[0m" : "";
    const char *bold = caps.color ? "\033[1m" : "";

    fprintf(out, "%s%s%s: %s\n",
            error_color, ex->type, reset, ex->message);

    // Source location
    if (ex->source_file)
    {
        fprintf(out, "  at %s%s:%d%s\n",
                bold, ex->source_file, ex->source_line, reset);
    }

    // Error code
    if (ex->error_code != 0)
    {
        fputs("  ", out);
        apep_fputs_utf8(out, _("Error Code:"));
        fprintf(out, " %d", ex->error_code);

        // Try to get errno description
#ifdef _WIN32
        if (ex->error_code > 0 && ex->error_code < 256)
        {
            char err_buf[256];
            if (strerror_s(err_buf, sizeof(err_buf), ex->error_code) == 0)
            {
                fprintf(out, " (%s)", err_buf);
            }
        }
#else
        if (ex->error_code > 0)
        {
            fprintf(out, " (%s)", strerror(ex->error_code));
        }
#endif
        fprintf(out, "\n");
    }

    // Stack trace
    if (ex->stack_trace)
    {
        fputs("  ", out);
        apep_fputs_utf8(out, _("Stack Trace:"));
        fputc('\n', out);
        print_stack_trace(out, (const apep_stack_trace_t *)ex->stack_trace, 4);
    }
}

void apep_exception_print_chain(const apep_options_t *opt, const apep_exception_t *ex, int max_depth)
{
    if (!ex)
        return;

    FILE *out = stdout;

    int depth = 0;
    const apep_exception_t *current = ex;

    while (current)
    {
        if (max_depth > 0 && depth >= max_depth)
            break;

        if (depth > 0)
        {
            fputc('\n', out);
            apep_fputs_utf8(out, _("Caused by:"));
            fputc('\n', out);
        }

        apep_exception_print(opt, current);

        current = current->inner;
        depth++;
    }
}

void apep_exception_destroy(apep_exception_t *ex)
{
    if (!ex)
        return;

    // Free message
    if (ex->owns_message && ex->message)
    {
        free(ex->message);
    }

    // Free stack trace
    if (ex->stack_trace)
    {
        apep_stack_trace_t *stack = (apep_stack_trace_t *)ex->stack_trace;
#if !defined(_WIN32) || defined(__GNUC__)
        if (stack->symbols)
        {
            free(stack->symbols);
        }
#endif
        free(stack);
    }

    // Free inner exception
    if (ex->owns_inner && ex->inner)
    {
        apep_exception_destroy(ex->inner);
    }

    free(ex);
}
