#include "../include/apep/apep.h"

int main(void)
{
    const char *input =
        "(1+)\n";

    apep_options_t opt;
    apep_options_default(&opt);
    opt.out = stderr;

    apep_text_source_t src = apep_text_source_from_string("input.expr", input);

    apep_note_t notes[] = {
        {"hint", "remove ')' or add a number after '+'."}};

    apep_print_text_diagnostic(
        &opt,
        APEP_SEV_ERROR,
        "E_SYNTAX",
        "unexpected token ')'",
        &src,
        (apep_loc_t){1, 4},
        1,
        notes,
        1);

    return 0;
}
