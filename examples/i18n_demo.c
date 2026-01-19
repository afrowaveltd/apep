#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include "../include/apep/apep_i18n.h"

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

int main(void)
{
    /* Initialize localization with detected system locale */
    apep_i18n_init(NULL, find_locales_dir());

    printf("Current locale: %s\n\n", apep_i18n_get_locale());

    /* Example 1: Simple error */
    apep_options_t opt;
    apep_options_default(&opt);
    opt.out = stderr;

    const char *input = "(1+)\n";
    apep_text_source_t src = apep_text_source_from_string("input.expr", input);

    apep_note_t notes[] = {
        {_("hint"), _("remove ')' or add a number after '+'.")}};

    apep_print_text_diagnostic(
        &opt,
        APEP_SEV_ERROR,
        "E_SYNTAX",
        _("unexpected token ')'"),
        &src,
        (apep_loc_t){1, 4},
        1,
        notes,
        1);

    printf("\n--- Testing different locales ---\n\n");

    /* Test Czech locale */
    printf("Switching to Czech locale...\n");
    apep_i18n_set_locale("cs");
    printf("Current locale: %s\n\n", apep_i18n_get_locale());

    apep_print_text_diagnostic(
        &opt,
        APEP_SEV_ERROR,
        "E_SYNTAX",
        _("unexpected token ')'"),
        &src,
        (apep_loc_t){1, 4},
        1,
        notes,
        1);

    /* Test English locale */
    printf("\n\nSwitching to English locale...\n");
    apep_i18n_set_locale("en");
    printf("Current locale: %s\n\n", apep_i18n_get_locale());

    apep_print_text_diagnostic(
        &opt,
        APEP_SEV_WARN,
        "W_UNUSED",
        _("unused variable 'x'"),
        &src,
        (apep_loc_t){1, 2},
        1,
        NULL,
        0);

    /* Test helper functions with localization */
    printf("\n\n--- Testing helper functions ---\n\n");

    apep_error_simple(&opt, "E_TEST", _("This is a test error"), _("Check your configuration"));

    apep_error_file(&opt, "config.txt", _("read"), _("file not found"));

    /* Cleanup */
    apep_i18n_cleanup();

    return 0;
}
