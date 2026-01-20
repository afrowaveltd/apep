#include "../include/apep/apep.h"
#include "../include/apep/apep_i18n.h"
#include "../include/apep/apep_exception.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* Find locales directory relative to where we are */
static const char *find_locales_dir(void)
{
    /* Try current directory first */
    FILE *test = fopen("locales/en.loc", "r");
    if (test)
    {
        fclose(test);
        return "locales";
    }

    /* Try parent directory (when running from bin/) */
    test = fopen("../locales/en.loc", "r");
    if (test)
    {
        fclose(test);
        return "../locales";
    }

    /* Fallback to current directory anyway */
    return "locales";
}

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

int main(int argc, char **argv)
{
#ifdef _WIN32
    /* Enable UTF-8 output on Windows console */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    const char *lang = parse_lang_arg(argc, argv);
    const char *locales_dir = find_locales_dir();
    apep_i18n_init(lang, locales_dir);

    apep_options_t opt = {0};

    printf("%s\n", _("=== Standard Logging ==="));
    apep_print_message(&opt, APEP_LVL_TRACE, "BOOT", _("entering early init"));
    apep_print_message(&opt, APEP_LVL_DEBUG, "CFG", _("config loaded"));
    apep_print_message(&opt, APEP_LVL_INFO, "NET", _("connected"));
    apep_print_message(&opt, APEP_LVL_WARN, "IO", _("slow response detected"));
    apep_print_message(&opt, APEP_LVL_ERROR, "FS", _("open failed"));
    apep_print_message(&opt, APEP_LVL_CRITICAL, "SYS", _("out of memory"));

    printf("\n%s\n", _("=== Exception Logging ==="));

    /* Logger can also handle exceptions */
    apep_options_default(&opt);

    /* Example: Database error with exception */
    printf("\n%s\n", _("Database Error:"));
    apep_exception_t *db_ex = apep_exception_create(
        "SQLException",
        _("Query failed: table 'users' does not exist"));
    apep_exception_set_source(db_ex, "db_manager.c", 234);
    apep_exception_set_code(db_ex, -1);
    apep_exception_capture_stack(db_ex);
    apep_exception_print(&opt, db_ex);
    apep_exception_destroy(db_ex);

    /* Example: Network error chain */
    printf("\n%s\n", _("Network Error Chain:"));
    apep_exception_t *timeout = apep_exception_create(
        "TimeoutException",
        _("Connection timed out after 30 seconds"));
    apep_exception_set_source(timeout, "socket.c", 412);

    apep_exception_t *conn_ex = apep_exception_create(
        "ConnectionException",
        _("Failed to establish connection to server 192.168.1.100:5432"));
    apep_exception_set_source(conn_ex, "connection.c", 156);
    apep_exception_set_inner(conn_ex, timeout);
    apep_exception_capture_stack(conn_ex);

    apep_exception_print_chain(&opt, conn_ex, 0);
    apep_exception_destroy(conn_ex);

    printf("\n%s\n", _("=== Logger with exceptions provides structured error tracking ==="));
    return 0;
}
