#include "../include/apep/apep.h"

int main(void)
{
    uint8_t buf[64];
    for (size_t i = 0; i < sizeof(buf); i++)
        buf[i] = (uint8_t)i;

    apep_options_t opt;
    apep_options_default(&opt);
    opt.out = stderr;

    apep_note_t notes[] = {
        {"note", "This is a demo. Hexdump rendering will be added next."}};

    apep_print_hex_diagnostic(
        &opt,
        APEP_SEV_ERROR,
        "E_BIN",
        "checksum mismatch",
        "payload.bin",
        buf,
        sizeof(buf),
        (apep_span_t){0x1A, 4},
        notes,
        1);

    return 0;
}
