#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include "../include/apep/apep_i18n.h"
#include "common_i18n.h"

int main(int argc, char **argv)
{
    demo_i18n_init(argc, argv, "apep_helpers_demo");

    /* Setup global options once */
    apep_options_t opt;
    apep_options_default(&opt);
    opt.out = stderr;
    apep_set_global_options(&opt);

    printf("%s\n\n", _("title"));

    /* 1. Quick logging macros */
    printf("%s\n", _("sec1"));
    APEP_LOG_INFO("APP", _("app_started"));
    APEP_LOG_WARN("CONFIG", _("default_config"));
    APEP_LOG_ERROR("NET", _("connection_timeout"));

    printf("\n%s\n", _("sec2"));
    apep_print_message_fmt(NULL, APEP_LVL_INFO, "USER",
                           _("user_login"), "alice", "192.168.1.100");
    apep_print_message_fmt(NULL, APEP_LVL_WARN, "CACHE",
                           _("cache_usage"), 85);

    printf("\n%s\n", _("sec3"));
    apep_error_simple(NULL, "E0001", _("invalid_config"), _("check_config"));

    apep_error_simple_fmt(NULL, "E0042",
                          _("expected_args"), 3, 1);

    printf("\n%s\n", _("sec4"));
    apep_error_file(NULL, "/etc/myapp/config.ini", _("open"), _("permission_denied"));

    printf("\n%s\n", _("sec5"));
    {
        const char *code = "x = lenght + 1\n";
        apep_text_source_t src = apep_text_source_from_string("script.py", code);
        apep_loc_t loc = {1, 5};

        apep_error_unknown_identifier(NULL, "lenght", "length", &src, loc);
    }

    printf("\n%s\n", _("sec6"));
    apep_error_assert(NULL, "ptr != NULL", __FILE__, __LINE__);

    printf("\n%s\n", _("sec7"));
    APEP_DEBUG("INTERNAL", _("debug_msg"));

    printf("\n%s\n", _("complete"));

    apep_i18n_cleanup();
    return 0;
}
