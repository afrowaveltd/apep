#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include "../include/apep/apep_i18n.h"
#include <stdio.h>

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

void print_separator(void)
{
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("\n");
}

void demo_text_diagnostics(apep_options_t *opt, const char *locale_name)
{
    printf("Text Diagnostics in %s:\n\n", locale_name);

    const char *input = "(1+)\nlet x = 42;\n";
    apep_text_source_t src = apep_text_source_from_string("example.code", input);

    // Error with hint
    apep_note_t notes[] = {
        {_("hint"), _("remove ')' or add a number after '+'.")}};

    apep_print_text_diagnostic(
        opt, APEP_SEV_ERROR, "E_SYNTAX",
        _("unexpected token ')'"),
        &src, (apep_loc_t){1, 4}, 1,
        notes, 1);

    printf("\n");

    // Warning
    apep_print_text_diagnostic(
        opt, APEP_SEV_WARN, "W_UNUSED",
        _("unused variable 'x'"),
        &src, (apep_loc_t){2, 5}, 1,
        NULL, 0);
}

void demo_helper_functions(apep_options_t *opt, const char *locale_name)
{
    printf("\n\nHelper Functions in %s:\n\n", locale_name);

    // Simple error
    apep_error_simple(opt, "E_TEST",
                      _("This is a test error"),
                      _("Check your configuration"));

    printf("\n");

    // File error
    apep_error_file(opt, "config.txt", "read", "file not found");

    printf("\n");

    // Unknown identifier
    const char *code = "foo\nbar\n";
    apep_text_source_t src = apep_text_source_from_string("test.txt", code);
    apep_error_unknown_identifier(opt, "foo", "bar", &src, (apep_loc_t){1, 1});
}

void demo_log_levels(apep_options_t *opt, const char *locale_name)
{
    printf("\n\nLog Levels in %s:\n\n", locale_name);

    apep_print_message(opt, APEP_LVL_TRACE, "SYS", _("This is a test error"));
    apep_print_message(opt, APEP_LVL_DEBUG, "NET", _("Check your configuration"));
    apep_print_message(opt, APEP_LVL_INFO, "APP", _("This is a test error"));
    apep_print_message(opt, APEP_LVL_WARN, "DB", _("file not found"));
    apep_print_message(opt, APEP_LVL_ERROR, "IO", _("unknown error"));
    apep_print_message(opt, APEP_LVL_CRITICAL, "CORE", _("This is a test error"));
}

void demo_severity_levels(const char *locale_name)
{
    printf("\n\nSeverity Level Names in %s:\n\n", locale_name);

    printf("  APEP_SEV_ERROR   -> %s\n", apep_severity_name(APEP_SEV_ERROR));
    printf("  APEP_SEV_WARN    -> %s\n", apep_severity_name(APEP_SEV_WARN));
    printf("  APEP_SEV_NOTE    -> %s\n", apep_severity_name(APEP_SEV_NOTE));
}

int main(void)
{
    apep_options_t opt;
    apep_options_default(&opt);
    opt.out = stdout;

    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║           APEP Localization Comprehensive Demo               ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");

    // Find locales directory
    const char *locales_dir = find_locales_dir();

    // Detect system locale
    const char *sys_locale = apep_i18n_detect_system_locale();
    printf("\nDetected system locale: %s\n", sys_locale);

    print_separator();

    // Demo in English
    printf("┌───────────────────────────────────────────────────────────────┐\n");
    printf("│                     ENGLISH LOCALE (en)                       │\n");
    printf("└───────────────────────────────────────────────────────────────┘\n\n");

    apep_i18n_init("en", locales_dir);
    printf("Active locale: %s\n\n", apep_i18n_get_locale());

    demo_severity_levels("English");
    demo_text_diagnostics(&opt, "English");
    demo_helper_functions(&opt, "English");
    demo_log_levels(&opt, "English");

    print_separator();

    // Demo in Czech
    printf("┌───────────────────────────────────────────────────────────────┐\n");
    printf("│                     CZECH LOCALE (cs)                         │\n");
    printf("└───────────────────────────────────────────────────────────────┘\n\n");

    apep_i18n_set_locale("cs");
    printf("Active locale: %s\n\n", apep_i18n_get_locale());

    demo_severity_levels("Czech");
    demo_text_diagnostics(&opt, "Czech");
    demo_helper_functions(&opt, "Czech");
    demo_log_levels(&opt, "Czech");

    print_separator();

    // Demo fallback behavior
    printf("┌───────────────────────────────────────────────────────────────┐\n");
    printf("│                  FALLBACK DEMONSTRATION                       │\n");
    printf("└───────────────────────────────────────────────────────────────┘\n\n");

    printf("Requesting non-existent key:\n");
    printf("  _(\"NonExistentKey\") = \"%s\"\n", _("NonExistentKey"));
    printf("\nThis demonstrates fallback to key itself when translation missing.\n");

    print_separator();

    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                         Demo Complete                         ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n\n");

    // Cleanup
    apep_i18n_cleanup();

    return 0;
}
