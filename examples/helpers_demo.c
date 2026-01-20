#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include "../include/apep/apep_i18n.h"
#include "common_i18n.h"

int main(int argc, char **argv)
{
    demo_i18n_init(argc, argv, "helpers_demo");

    /* Setup global options once */
    apep_options_t opt;
    apep_options_default(&opt);
    opt.out = stderr;
    apep_set_global_options(&opt);

    printf("%s\n\n", _("=== APEP Helpers Demo ==="));

    /* 1. Quick logging macros */
    printf("%s\n", _("1. Quick logging macros:"));
    APEP_LOG_INFO("APP", _("application started successfully"));
    APEP_LOG_WARN("CONFIG", _("using default configuration"));
    APEP_LOG_ERROR("NET", _("connection timeout"));

    printf("\n%s\n", _("2. Formatted messages:"));
    apep_print_message_fmt(NULL, APEP_LVL_INFO, "USER",
                           _("user '%s' logged in from %s"), "alice", "192.168.1.100");
    apep_print_message_fmt(NULL, APEP_LVL_WARN, "CACHE",
                           _("cache usage at %d%% capacity"), 85);

    printf("\n%s\n", _("3. Simple errors:"));
    apep_error_simple(NULL, "E0001", _("invalid configuration"), _("check config.ini file"));

    apep_error_simple_fmt(NULL, "E0042",
                          _("expected %d arguments, got %d"), 3, 1);

    printf("\n%s\n", _("4. File errors:"));
    apep_error_file(NULL, "/etc/myapp/config.ini", _("open"), _("permission denied"));

    printf("\n%s\n", _("5. Unknown identifier with suggestion:"));
    {
        const char *code = "x = lenght + 1\n";
        apep_text_source_t src = apep_text_source_from_string("script.py", code);
        apep_loc_t loc = {1, 5};

        apep_error_unknown_identifier(NULL, "lenght", "length", &src, loc);
    }

    printf("\n%s\n", _("6. Assert failure:"));
    apep_error_assert(NULL, "ptr != NULL", __FILE__, __LINE__);

    printf("\n%s\n", _("7. Debug logging (compiled out in release):"));
    APEP_DEBUG("INTERNAL", _("this message only appears in debug builds"));

    printf("\n%s\n", _("=== Demo Complete ==="));

    apep_i18n_cleanup();
    return 0;
}
