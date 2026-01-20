#include "../include/apep/apep.h"
#include "../include/apep/apep_i18n.h"
#include "common_i18n.h"

int main(int argc, char **argv)
{
    demo_i18n_init(argc, argv, "log_demo");

    apep_options_t opt = {0};

    apep_print_message(&opt, APEP_LVL_TRACE, "BOOT", _("entering early init"));
    apep_print_message(&opt, APEP_LVL_DEBUG, "CFG", _("config loaded"));
    apep_print_message(&opt, APEP_LVL_INFO, "NET", _("connected"));
    apep_print_message(&opt, APEP_LVL_WARN, "IO", _("slow response detected"));
    apep_print_message(&opt, APEP_LVL_ERROR, "FS", _("open failed"));
    apep_print_message(&opt, APEP_LVL_CRITICAL, "SYS", _("out of memory"));

    apep_i18n_cleanup();
    return 0;
}
