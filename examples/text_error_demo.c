#include "../include/apep/apep.h"
#include "../include/apep/apep_i18n.h"
#include "common_i18n.h"

int main(int argc, char **argv)
{
    demo_i18n_init(argc, argv, "text_error_demo");

    const char *input =
        "(1+)\n";

    apep_options_t opt;
    apep_options_default(&opt);
    opt.out = stderr;

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

    apep_i18n_cleanup();
    return 0;
}
