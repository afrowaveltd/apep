#include "../include/apep/apep_i18n.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <locale.h>
#endif

/* ----------------------------
Internal structures
---------------------------- */

typedef struct i18n_entry
{
    char *key;
    char *value;
    struct i18n_entry *next;
} i18n_entry_t;

#define I18N_HASH_SIZE 256

typedef struct
{
    char locale[16];
    char locales_dir[256];
    i18n_entry_t *hash_table[I18N_HASH_SIZE];
    int initialized;
} i18n_context_t;

static i18n_context_t g_i18n = {0};

/* ----------------------------
Hash function
---------------------------- */

static unsigned int i18n_hash(const char *key)
{
    unsigned int hash = 5381;
    int c;

    while ((c = (unsigned char)*key++))
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash % I18N_HASH_SIZE;
}

/* ----------------------------
Entry management
---------------------------- */

static void i18n_free_entry(i18n_entry_t *entry)
{
    if (!entry)
        return;

    if (entry->key)
        free(entry->key);
    if (entry->value)
        free(entry->value);
    free(entry);
}

static void i18n_clear_table(void)
{
    for (int i = 0; i < I18N_HASH_SIZE; i++)
    {
        i18n_entry_t *entry = g_i18n.hash_table[i];
        while (entry)
        {
            i18n_entry_t *next = entry->next;
            i18n_free_entry(entry);
            entry = next;
        }
        g_i18n.hash_table[i] = NULL;
    }
}

static void i18n_add_entry(const char *key, const char *value)
{
    if (!key || !value)
        return;

    unsigned int idx = i18n_hash(key);

    /* Check if key already exists */
    i18n_entry_t *entry = g_i18n.hash_table[idx];
    while (entry)
    {
        if (strcmp(entry->key, key) == 0)
        {
            /* Update existing value */
            free(entry->value);
            entry->value = strdup(value);
            return;
        }
        entry = entry->next;
    }

    /* Create new entry */
    i18n_entry_t *new_entry = (i18n_entry_t *)malloc(sizeof(i18n_entry_t));
    if (!new_entry)
        return;

    new_entry->key = strdup(key);
    new_entry->value = strdup(value);
    new_entry->next = g_i18n.hash_table[idx];
    g_i18n.hash_table[idx] = new_entry;
}

static const char *i18n_find_entry(const char *key)
{
    if (!key)
        return NULL;

    unsigned int idx = i18n_hash(key);
    i18n_entry_t *entry = g_i18n.hash_table[idx];

    while (entry)
    {
        if (strcmp(entry->key, key) == 0)
        {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

/* ----------------------------
Parse .loc file
---------------------------- */

static char *trim_whitespace(char *str)
{
    char *end;

    /* Trim leading space */
    while (*str == ' ' || *str == '\t' || *str == '\r' || *str == '\n')
        str++;

    if (*str == 0)
        return str;

    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n'))
        end--;

    end[1] = '\0';

    return str;
}

static char *find_unquoted_colon(char *s)
{
    int in_quotes = 0;
    int escaped = 0;

    for (char *p = s; *p; p++)
    {
        if (escaped)
        {
            escaped = 0;
            continue;
        }

        if (in_quotes && *p == '\\')
        {
            escaped = 1;
            continue;
        }

        if (*p == '"')
        {
            in_quotes = !in_quotes;
            continue;
        }

        if (!in_quotes && *p == ':')
            return p;
    }

    return NULL;
}

static int utf8_append_codepoint(char *out, size_t out_cap, size_t *out_len, unsigned int cp)
{
    if (cp <= 0x7F)
    {
        if (*out_len + 1 >= out_cap)
            return -1;
        out[(*out_len)++] = (char)cp;
    }
    else if (cp <= 0x7FF)
    {
        if (*out_len + 2 >= out_cap)
            return -1;
        out[(*out_len)++] = (char)(0xC0 | (cp >> 6));
        out[(*out_len)++] = (char)(0x80 | (cp & 0x3F));
    }
    else if (cp <= 0xFFFF)
    {
        if (*out_len + 3 >= out_cap)
            return -1;
        out[(*out_len)++] = (char)(0xE0 | (cp >> 12));
        out[(*out_len)++] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[(*out_len)++] = (char)(0x80 | (cp & 0x3F));
    }
    else if (cp <= 0x10FFFF)
    {
        if (*out_len + 4 >= out_cap)
            return -1;
        out[(*out_len)++] = (char)(0xF0 | (cp >> 18));
        out[(*out_len)++] = (char)(0x80 | ((cp >> 12) & 0x3F));
        out[(*out_len)++] = (char)(0x80 | ((cp >> 6) & 0x3F));
        out[(*out_len)++] = (char)(0x80 | (cp & 0x3F));
    }
    else
    {
        return -1;
    }

    return 0;
}

static int parse_hex4(const char *p, unsigned int *out)
{
    unsigned int v = 0;
    for (int i = 0; i < 4; i++)
    {
        unsigned char c = (unsigned char)p[i];
        if (!isxdigit(c))
            return -1;
        v <<= 4;
        if (c >= '0' && c <= '9')
            v |= (c - '0');
        else if (c >= 'a' && c <= 'f')
            v |= (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F')
            v |= (c - 'A' + 10);
    }
    *out = v;
    return 0;
}

static char *unescape_json_like(const char *src, size_t len)
{
    if (!src)
        return NULL;

    size_t out_cap = (len * 4) + 1;
    char *out = (char *)malloc(out_cap);
    if (!out)
        return NULL;

    size_t out_len = 0;
    for (size_t i = 0; i < len; i++)
    {
        char c = src[i];
        if (c != '\\')
        {
            if (out_len + 1 >= out_cap)
                break;
            out[out_len++] = c;
            continue;
        }

        if (i + 1 >= len)
        {
            if (out_len + 1 >= out_cap)
                break;
            out[out_len++] = '\\';
            continue;
        }

        char esc = src[++i];
        switch (esc)
        {
        case '"':
            out[out_len++] = '"';
            break;
        case '\\':
            out[out_len++] = '\\';
            break;
        case '/':
            out[out_len++] = '/';
            break;
        case 'b':
            out[out_len++] = '\b';
            break;
        case 'f':
            out[out_len++] = '\f';
            break;
        case 'n':
            out[out_len++] = '\n';
            break;
        case 'r':
            out[out_len++] = '\r';
            break;
        case 't':
            out[out_len++] = '\t';
            break;
        case 'u':
        {
            if (i + 4 <= len)
            {
                unsigned int cp = 0;
                if (parse_hex4(src + i + 1, &cp) == 0)
                {
                    i += 4;

                    if (cp >= 0xD800 && cp <= 0xDBFF && i + 6 < len && src[i + 1] == '\\' && src[i + 2] == 'u')
                    {
                        unsigned int low = 0;
                        if (parse_hex4(src + i + 3, &low) == 0 && low >= 0xDC00 && low <= 0xDFFF)
                        {
                            i += 6;
                            cp = 0x10000 + (((cp - 0xD800) << 10) | (low - 0xDC00));
                        }
                    }

                    if (utf8_append_codepoint(out, out_cap, &out_len, cp) == 0)
                        break;
                }
            }
            /* fallback to literal sequence if invalid */
            out[out_len++] = '\\';
            out[out_len++] = 'u';
            break;
        }
        default:
            /* Preserve unknown escape literally */
            out[out_len++] = esc;
            break;
        }
    }

    out[out_len] = '\0';
    return out;
}

static int parse_quoted_token(const char *p, const char **end, char **out)
{
    if (!p || *p != '"')
        return -1;

    int escaped = 0;
    const char *start = p + 1;
    const char *q = start;
    for (; *q; q++)
    {
        if (escaped)
        {
            escaped = 0;
            continue;
        }
        if (*q == '\\')
        {
            escaped = 1;
            continue;
        }
        if (*q == '"')
            break;
    }

    if (*q != '"')
        return -1;

    size_t len = (size_t)(q - start);
    char *unescaped = unescape_json_like(start, len);
    if (!unescaped)
        return -1;

    *end = q + 1;
    *out = unescaped;
    return 0;
}

static int i18n_parse_line(char *line)
{
    /* Skip empty lines and comments */
    char *trimmed = trim_whitespace(line);
    if (trimmed[0] == '\0' || trimmed[0] == '#')
        return 0;

    /* Skip JSON braces (for flat JSON support) */
    if (trimmed[0] == '{' || trimmed[0] == '}')
        return 0;

    /* Find ':' outside quotes */
    char *colon = find_unquoted_colon(trimmed);
    if (!colon)
        return -1; /* Invalid format */

    /* Split key and value */
    *colon = '\0';
    char *key_raw = trim_whitespace(trimmed);
    char *value_raw = trim_whitespace(colon + 1);

    char *key = NULL;
    char *value = NULL;

    if (key_raw[0] == '"')
    {
        const char *end = NULL;
        if (parse_quoted_token(key_raw, &end, &key) != 0)
            return -1;
    }
    else
    {
        key = strdup(key_raw);
    }

    if (!key)
        return -1;

    if (value_raw[0] == '"')
    {
        const char *end = NULL;
        if (parse_quoted_token(value_raw, &end, &value) != 0)
        {
            free(key);
            return -1;
        }
        /* Allow trailing comma/whitespace after quoted value */
        if (end)
        {
            const char *p = end;
            while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
                p++;
            if (*p == ',')
            {
                p++;
                while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
                    p++;
            }
            /* Any remaining trailing content is ignored for robustness */
        }
    }
    else
    {
        /* Remove trailing comma if present */
        size_t value_len = strlen(value_raw);
        if (value_len > 0 && value_raw[value_len - 1] == ',')
        {
            value_raw[value_len - 1] = '\0';
            value_raw = trim_whitespace(value_raw);
        }
        value = strdup(value_raw);
    }

    i18n_add_entry(key, value);
    free(key);
    free(value);
    return 0;
}

static int i18n_load_locale_file(const char *filepath)
{
    FILE *f = fopen(filepath, "r");
    if (!f)
        return -1;

#ifdef _WIN32
    /* Set console to UTF-8 for proper display */
    SetConsoleOutputCP(CP_UTF8);
#endif

    char line[2048];
    int line_num = 0;

    while (fgets(line, sizeof(line), f))
    {
        line_num++;
        int result = i18n_parse_line(line);
        if (result < 0)
        {
            fprintf(stderr, "Warning: invalid format in %s at line %d\n", filepath, line_num);
        }
    }

    fclose(f);
    return 0;
}

/* ----------------------------
System locale detection
---------------------------- */

const char *apep_i18n_detect_system_locale(void)
{
#ifdef _WIN32
    /* Windows: use GetUserDefaultLCID and GetLocaleInfoA */
    static char result[16];
    LCID lcid = GetUserDefaultLCID();

    /* Get the ISO 639 language code */
    if (GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME, result, sizeof(result)) > 0)
    {
        return result;
    }
#else
    /* Unix/Linux: use LANG environment variable */
    const char *lang = getenv("LANG");
    if (lang)
    {
        static char result[16];
        strncpy(result, lang, sizeof(result) - 1);
        result[sizeof(result) - 1] = '\0';

        /* Extract language code (e.g., "en_US.UTF-8" -> "en") */
        char *underscore = strchr(result, '_');
        if (underscore)
            *underscore = '\0';

        char *dot = strchr(result, '.');
        if (dot)
            *dot = '\0';

        return result;
    }
#endif

    return "en"; /* Default fallback */
}

/* ----------------------------
Public API
---------------------------- */

int apep_i18n_init(const char *locale, const char *locales_dir)
{
    if (g_i18n.initialized)
    {
        i18n_clear_table();
    }

    /* Set locale */
    const char *loc = locale;
    if (!loc || loc[0] == '\0')
    {
        loc = apep_i18n_detect_system_locale();
    }

    strncpy(g_i18n.locale, loc, sizeof(g_i18n.locale) - 1);
    g_i18n.locale[sizeof(g_i18n.locale) - 1] = '\0';

    /* Set locales directory */
    const char *dir = locales_dir ? locales_dir : "locales";
    strncpy(g_i18n.locales_dir, dir, sizeof(g_i18n.locales_dir) - 1);
    g_i18n.locales_dir[sizeof(g_i18n.locales_dir) - 1] = '\0';

    /* Build locale file path - try .json first, then .loc */
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s.json", g_i18n.locales_dir, g_i18n.locale);

    /* Try to load locale file */
    if (i18n_load_locale_file(filepath) < 0)
    {
        /* Try .loc extension */
        snprintf(filepath, sizeof(filepath), "%s/%s.loc", g_i18n.locales_dir, g_i18n.locale);
        if (i18n_load_locale_file(filepath) < 0)
        {
            /* Fallback to English */
            if (strcmp(g_i18n.locale, "en") != 0)
            {
                snprintf(filepath, sizeof(filepath), "%s/en.json", g_i18n.locales_dir);
                if (i18n_load_locale_file(filepath) < 0)
                {
                    snprintf(filepath, sizeof(filepath), "%s/en.loc", g_i18n.locales_dir);
                    i18n_load_locale_file(filepath); /* Ignore error */
                }
            }
        }
    }

    g_i18n.initialized = 1;
    return 0;
}

const char *apep_i18n_get(const char *key)
{
    if (!key)
        return "";

    /* If not initialized, return key as fallback */
    if (!g_i18n.initialized)
    {
        return key;
    }

    const char *value = i18n_find_entry(key);
    if (value)
    {
        return value;
    }

    /* Fallback: return key itself */
    return key;
}

int apep_i18n_set_locale(const char *locale)
{
    if (!locale)
        return -1;

    return apep_i18n_init(locale, g_i18n.locales_dir);
}

const char *apep_i18n_get_locale(void)
{
    if (!g_i18n.initialized)
        return "en";

    return g_i18n.locale;
}

void apep_i18n_cleanup(void)
{
    if (!g_i18n.initialized)
        return;

    i18n_clear_table();
    g_i18n.initialized = 0;
    g_i18n.locale[0] = '\0';
}
