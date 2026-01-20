#include "../include/apep/apep_i18n.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <locale.h>
#endif

static int ensure_dir(const char *path)
{
#ifdef _WIN32
    if (_mkdir(path) == 0 || errno == EEXIST)
        return 0;
#else
    if (mkdir(path, 0755) == 0 || errno == EEXIST)
        return 0;
#endif
    return -1;
}

static int write_stress_locale(const char *dir)
{
    if (ensure_dir(dir) != 0)
        return -1;

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/en.loc", dir);

    FILE *f = fopen(filepath, "wb");
    if (!f)
        return -1;

    fputs("# Stress-test locale with escapes and UTF-8\n", f);
    fputs("\n", f);
    fputs("\"simple\":\"Hello\"\n", f);
    fputs("\"colon:key\":\"value:with:colon\"\n", f);
    fputs("\"escaped_quote\":\"He said \\\"hi\\\"\"\n", f);
    fputs("\"backslash\":\"C:\\\\path\\\\file\"\n", f);
    fputs("\"tab\":\"A\\tB\"\n", f);
    fputs("\"newline\":\"Line1\\nLine2\"\n", f);
    fputs("\"unicode\":\"český řetězec\"\n", f);
    fputs("\"u_escape\":\"\\u010D\\u0159\\u017E\"\n", f);
    fputs("\"surrogate\":\"\\uD83D\\uDE80\"\n", f);
    fputs("\"trailing_comma\":\"works\",\n", f);
    fputs("\n", f);
    fputs("# Invalid lines that must be ignored safely\n", f);
    fputs("invalid line without colon\n", f);
    fputs("\"unterminated\":\"value\n", f);
    fputs("\"bad_escape\":\"\\u12G4\"\n", f);

    fclose(f);
    return 0;
}

static void print_visible(const char *key, const char *value)
{
    printf("%s = ", key);
    if (!value)
    {
        printf("(null)\n");
        return;
    }

    printf("\"");
    for (const unsigned char *p = (const unsigned char *)value; *p; p++)
    {
        unsigned char c = *p;
        switch (c)
        {
        case '\n':
            printf("\\n");
            break;
        case '\t':
            printf("\\t");
            break;
        case '\r':
            printf("\\r");
            break;
        case '\b':
            printf("\\b");
            break;
        case '\f':
            printf("\\f");
            break;
        case '\\':
            printf("\\\\");
            break;
        case '"':
            printf("\\\"");
            break;
        default:
            if (c < 32)
                printf("\\x%02X", c);
            else
                printf("%c", c);
            break;
        }
    }
    printf("\"\n");
}

static const char *parse_lang_arg(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--lang") == 0 && i + 1 < argc)
        {
            return argv[i + 1];
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
#ifdef _WIN32
    /* Enable UTF-8 output on Windows console */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#else
    setlocale(LC_ALL, "");
#endif

    const char *dir = "stress_locales";
    if (write_stress_locale(dir) != 0)
    {
        fprintf(stderr, "Failed to write stress locale: %s\n", strerror(errno));
        return 1;
    }

    const char *lang = parse_lang_arg(argc, argv);
    if (apep_i18n_init(lang, dir) != 0)
    {
        fprintf(stderr, "Failed to init i18n\n");
        return 1;
    }

    printf("APEP i18n Stress Demo\n");
    printf("=====================\n\n");
    printf("Loaded locale from: %s\n", dir);
    printf("Active locale: %s\n\n", apep_i18n_get_locale());

    const char *keys[] = {
        "simple",
        "colon:key",
        "escaped_quote",
        "backslash",
        "tab",
        "newline",
        "unicode",
        "u_escape",
        "surrogate",
        "trailing_comma",
        "missing_key"};

    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
    {
        const char *k = keys[i];
        const char *v = apep_i18n_get(k);
        print_visible(k, v);
    }

    printf("\nIf invalid lines were present, the parser should warn but continue safely.\n");

    apep_i18n_cleanup();

    /* Best-effort cleanup (safe to ignore failures) */
    {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "%s/en.loc", dir);
        remove(filepath);
    }
#ifdef _WIN32
    _rmdir(dir);
#else
    rmdir(dir);
#endif

    return 0;
}
