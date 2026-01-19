#include "../include/apep/apep.h"

int main(void)
{
    apep_options_t opt = {0};

    apep_print_message(&opt, APEP_LVL_TRACE, "BOOT", "entering early init");
    apep_print_message(&opt, APEP_LVL_DEBUG, "CFG", "config loaded");
    apep_print_message(&opt, APEP_LVL_INFO, "NET", "connected");
    apep_print_message(&opt, APEP_LVL_WARN, "IO", "slow response detected");
    apep_print_message(&opt, APEP_LVL_ERROR, "FS", "open failed");
    apep_print_message(&opt, APEP_LVL_CRITICAL, "SYS", "out of memory");

    return 0;
}
