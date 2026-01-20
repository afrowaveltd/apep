/* Common i18n helper for all demos */
#ifndef COMMON_I18N_H
#define COMMON_I18N_H

#include "../include/apep/apep_i18n.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Parse command line for --lang argument */
static const char *parse_lang_arg(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--lang") == 0 && i + 1 < argc)
        {
            return argv[i + 1];
        }
    }
    return NULL; /* Use system locale */
}

/* Find locales directory relative to executable */
static const char *find_demo_locales_dir(const char *demo_name)
{
    static char path[480];
    static char testpath[512];

    /* Try current directory first (running from examples/) */
    snprintf(path, sizeof(path), "%s_locales", demo_name);
    snprintf(testpath, sizeof(testpath), "%s/en.json", path);
    FILE *test = fopen(testpath, "r");
    if (test)
    {
        fclose(test);
        return path;
    }

    /* Try looking for locales next to executable (running from bin/) */
    snprintf(path, sizeof(path), "../examples/%s_locales", demo_name);
    snprintf(testpath, sizeof(testpath), "%s/en.json", path);
    test = fopen(testpath, "r");
    if (test)
    {
        fclose(test);
        return path;
    }

    /* Fallback */
    snprintf(path, sizeof(path), "%s_locales", demo_name);
    return path;
}

/* Initialize i18n for demo with UTF-8 support */
static void demo_i18n_init(int argc, char **argv, const char *demo_name)
{
#ifdef _WIN32
    /* Enable UTF-8 output on Windows console */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    const char *lang = parse_lang_arg(argc, argv);
    const char *locales_dir = find_demo_locales_dir(demo_name);

    apep_i18n_init(lang, locales_dir);
}

#endif /* COMMON_I18N_H */
