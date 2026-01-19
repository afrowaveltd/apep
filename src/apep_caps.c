#if defined(_WIN32)
/* MinGW needs these for _isatty and _fileno */
#ifndef _CRT_DECLARE_NONSTDC_NAMES
#define _CRT_DECLARE_NONSTDC_NAMES 1
#endif
#endif

#if !defined(_WIN32)
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include "../include/apep/apep.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(_WIN32)
#include <windows.h>
#include <io.h>
/* MinGW provides both fileno and _fileno, but fileno is standard */
#ifdef __MINGW32__
#define APEP_ISATTY isatty
#define APEP_FILENO fileno
#else
#define APEP_ISATTY _isatty
#define APEP_FILENO _fileno
#endif
#else
#include <unistd.h>
#include <sys/ioctl.h>
#define APEP_ISATTY isatty
#define APEP_FILENO fileno
#endif

static int apep_env_is_set(const char *key)
{
    const char *v = getenv(key);
    return (v && v[0] != '\0');
}

static int apep_str_contains_utf8(const char *s)
{
    if (!s)
        return 0;
    return (strstr(s, "UTF-8") || strstr(s, "utf8") || strstr(s, "UTF8"));
}

static int apep_detect_is_tty(FILE *out)
{
    if (!out)
        return 0;
    int fd = APEP_FILENO(out);
    if (fd < 0)
        return 0;
    return APEP_ISATTY(fd) ? 1 : 0;
}

static int apep_detect_width(FILE *out, int fallback)
{
    (void)out;

#if defined(_WIN32)
    HANDLE h = GetStdHandle(STD_ERROR_HANDLE);
    if (h == INVALID_HANDLE_VALUE || h == NULL)
        return fallback;

    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(h, &info))
        return fallback;

    int w = (int)(info.srWindow.Right - info.srWindow.Left + 1);
    return (w > 0) ? w : fallback;
#else
    int fd = out ? APEP_FILENO(out) : -1;
    if (fd < 0)
        return fallback;

    struct winsize ws;
    if (ioctl(fd, TIOCGWINSZ, &ws) == 0)
    {
        if (ws.ws_col > 0)
            return (int)ws.ws_col;
    }
    return fallback;
#endif
}

static int apep_detect_color_auto(int is_tty)
{
    /* Only enable colors on TTY */
    if (!is_tty)
        return 0;

    /* Check for explicit color environment variables */
    if (apep_env_is_set("COLORTERM"))
        return 1;

    const char *term = getenv("TERM");
    if (term)
    {
        /* Common terminals that support color */
        if (strstr(term, "color") || strstr(term, "xterm") ||
            strstr(term, "screen") || strstr(term, "tmux") ||
            strcmp(term, "linux") == 0)
        {
            return 1;
        }
    }

    /* Default to colors on TTY (most modern terminals support it) */
    return 1;
}

static int apep_detect_unicode_auto(int is_tty)
{
    if (!is_tty)
        return 0;

    /* allow forcing ASCII */
    if (apep_env_is_set("APEP_ASCII"))
        return 0;

    /* Conservative: only enable when locale suggests UTF-8 */
    const char *lc_all = getenv("LC_ALL");
    const char *lc_ctype = getenv("LC_CTYPE");
    const char *lang = getenv("LANG");

    if (apep_str_contains_utf8(lc_all) || apep_str_contains_utf8(lc_ctype) || apep_str_contains_utf8(lang))
    {
        return 1;
    }
    return 0;
}

apep_caps_t apep_detect_caps(FILE *out, const apep_options_t *opt)
{
    apep_caps_t caps;
    caps.is_tty = apep_detect_is_tty(out);

    int width = 80;
    if (opt && opt->width_override > 0)
    {
        width = opt->width_override;
    }
    else
    {
        width = apep_detect_width(out, 80);
    }
    if (width < 20)
        width = 20; /* keep layouts sane */
    caps.width = width;

    /* Color */
    if (opt && opt->force_no_color)
    {
        caps.color = 0;
    }
    else if (apep_env_is_set("NO_COLOR"))
    {
        caps.color = 0;
    }
    else
    {
        apep_color_mode_t cm = opt ? opt->color : APEP_COLOR_AUTO;

        if (cm == APEP_COLOR_OFF)
        {
            caps.color = 0;
        }
        else if (cm == APEP_COLOR_ON)
        {
            /* On means: only if TTY (avoid ANSI in redirected output) */
            caps.color = caps.is_tty ? 1 : 0;
        }
        else
        {
            /* AUTO */
            if (apep_env_is_set("CI"))
            {
                caps.color = 0;
            }
            else
            {
                caps.color = apep_detect_color_auto(caps.is_tty);
            }
        }
    }

    /* Unicode */
    apep_unicode_mode_t um = opt ? opt->unicode : APEP_UNICODE_AUTO;
    if (um == APEP_UNICODE_OFF)
        caps.unicode = 0;
    else if (um == APEP_UNICODE_ON)
        caps.unicode = caps.is_tty ? 1 : 0;
    else
        caps.unicode = apep_detect_unicode_auto(caps.is_tty);

    if (opt && opt->force_ascii)
    {
        caps.unicode = 0;
    }

    return caps;
}