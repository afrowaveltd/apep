#include "../include/apep/apep_i18n.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

static int i18n_parse_line(char *line)
{
    /* Skip empty lines and comments */
    char *trimmed = trim_whitespace(line);
    if (trimmed[0] == '\0' || trimmed[0] == '#')
        return 0;

    /* Find first ':' */
    char *colon = strchr(trimmed, ':');
    if (!colon)
        return -1; /* Invalid format */

    /* Split key and value */
    *colon = '\0';
    char *key = trim_whitespace(trimmed);
    char *value = trim_whitespace(colon + 1);

    /* Remove quotes if present */
    if (key[0] == '"' && key[strlen(key) - 1] == '"')
    {
        key++;
        key[strlen(key) - 1] = '\0';
    }

    if (value[0] == '"' && value[strlen(value) - 1] == '"')
    {
        value++;
        value[strlen(value) - 1] = '\0';
    }

    i18n_add_entry(key, value);
    return 0;
}

static int i18n_load_locale_file(const char *filepath)
{
    FILE *f = fopen(filepath, "r");
    if (!f)
        return -1;

    char line[2048];
    int line_num = 0;

    while (fgets(line, sizeof(line), f))
    {
        line_num++;
        if (i18n_parse_line(line) < 0)
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

    /* Build locale file path */
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s.loc", g_i18n.locales_dir, g_i18n.locale);

    /* Try to load locale file */
    if (i18n_load_locale_file(filepath) < 0)
    {
        /* Fallback to English */
        if (strcmp(g_i18n.locale, "en") != 0)
        {
            snprintf(filepath, sizeof(filepath), "%s/en.loc", g_i18n.locales_dir);
            i18n_load_locale_file(filepath); /* Ignore error */
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
