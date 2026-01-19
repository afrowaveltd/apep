#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"

static apep_severity_t global_min_severity = APEP_SEV_NOTE; /* Show everything by default */

void apep_set_min_severity(apep_severity_t min_sev)
{
    global_min_severity = min_sev;
}

apep_severity_t apep_get_min_severity(void)
{
    return global_min_severity;
}

int apep_severity_passes_filter(apep_severity_t sev)
{
    /* Lower numeric values = higher severity (ERROR=0, WARN=1, NOTE=2) */
    return sev <= global_min_severity;
}
