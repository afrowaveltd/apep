#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"

int main(void)
{
    /* Setup global options once */
    apep_options_t opt;
    apep_options_default(&opt);
    opt.out = stderr;
    apep_set_global_options(&opt);

    printf("=== APEP Helpers Demo ===\n\n");

    /* 1. Quick logging macros */
    printf("1. Quick logging macros:\n");
    APEP_LOG_INFO("APP", "application started successfully");
    APEP_LOG_WARN("CONFIG", "using default configuration");
    APEP_LOG_ERROR("NET", "connection timeout");

    printf("\n2. Formatted messages:\n");
    apep_print_message_fmt(NULL, APEP_LVL_INFO, "USER",
                           "user '%s' logged in from %s", "alice", "192.168.1.100");
    apep_print_message_fmt(NULL, APEP_LVL_WARN, "CACHE",
                           "cache usage at %d%% capacity", 85);

    printf("\n3. Simple errors:\n");
    apep_error_simple(NULL, "E0001", "invalid configuration", "check config.ini file");

    apep_error_simple_fmt(NULL, "E0042",
                          "expected %d arguments, got %d", 3, 1);

    printf("\n4. File errors:\n");
    apep_error_file(NULL, "/etc/myapp/config.ini", "open", "permission denied");

    printf("\n5. Unknown identifier with suggestion:\n");
    {
        const char *code = "x = lenght + 1\n";
        apep_text_source_t src = apep_text_source_from_string("script.py", code);
        apep_loc_t loc = {1, 5};

        apep_error_unknown_identifier(NULL, "lenght", "length", &src, loc);
    }

    printf("\n6. Assert failure:\n");
    apep_error_assert(NULL, "ptr != NULL", __FILE__, __LINE__);

    printf("\n7. Debug logging (compiled out in release):\n");
    APEP_DEBUG("INTERNAL", "this message only appears in debug builds");

    printf("\n=== Demo Complete ===\n");
    return 0;
}
