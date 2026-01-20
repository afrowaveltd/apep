#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include "../include/apep/apep_i18n.h"
#include "common_i18n.h"
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

static int streq(const char *a, const char *b)
{
    return a && b && strcmp(a, b) == 0;
}

static void usage(void)
{
    puts(_("show_demo_title"));
    puts("");
    puts(_("usage_line"));
    puts("");
    puts(_("options_header"));
    puts(_("opt_no_color"));
    puts(_("opt_ascii"));
    puts(_("opt_plain"));
    puts(_("opt_help"));
    puts("");
}

static void print_separator(const char *title)
{
    printf("\n");
    printf("==================================================\n");
    printf("  %s\n", _(title));
    printf("==================================================\n");
    printf("\n");
}

int main(int argc, char **argv)
{
    demo_i18n_init(argc, argv, "show_demo");

    apep_options_t opt = (apep_options_t){0};

    for (int i = 1; i < argc; i++)
    {
        if (streq(argv[i], "--no-color"))
            opt.force_no_color = 1;
        else if (streq(argv[i], "--ascii"))
            opt.force_ascii = 1;
        else if (streq(argv[i], "--plain"))
        {
            opt.force_no_color = 1;
            opt.force_ascii = 1;
        }
        else if (streq(argv[i], "--help") || streq(argv[i], "-h"))
        {
            usage();
            apep_i18n_cleanup();
            return 0;
        }
        else
        {
            fprintf(stderr, _("unknown_option"), argv[i]);
            fprintf(stderr, "\n\n");
            usage();
            apep_i18n_cleanup();
            return 2;
        }
    }

    printf(_("apep_version"), 
           APEP_VERSION_MAJOR, APEP_VERSION_MINOR, APEP_VERSION_PATCH);
    printf("\n");

    /* ================================================== */
    print_separator("sep_1");

    apep_print_message(&opt, APEP_LVL_TRACE, "BOOT", _("msg_initializing"));
    apep_print_message(&opt, APEP_LVL_DEBUG, "CFG", _("msg_loaded_config"));
    apep_print_message(&opt, APEP_LVL_INFO, "NET", _("msg_listening"));
    apep_print_message(&opt, APEP_LVL_WARN, "DB", _("msg_pool_exhausted"));
    apep_print_message(&opt, APEP_LVL_ERROR, "AUTH", _("msg_invalid_creds"));
    apep_print_message(&opt, APEP_LVL_CRITICAL, "SYS", _("msg_alloc_failed"));

    /* ================================================== */
    print_separator("sep_2");

    {
        const char *src_text = "(1+)\n";
        apep_text_source_t src = apep_text_source_from_string("input.expr", src_text);

        apep_note_t notes[1];
        notes[0].kind = "hint";
        notes[0].message = _("hint_expected_expr");

        apep_loc_t loc = {1, 4};

        apep_print_text_diagnostic(
            &opt, APEP_SEV_ERROR, "E0001",
            _("err_unexpected_token"),
            &src, loc, 1, notes, 1);
    }

    /* ================================================== */
    print_separator("sep_3");

    {
        const char *src_text =
            "function calculate(x, y) {\n"
            "    let result = x + y\n"
            "    return result\n"
            "}\n";

        apep_text_source_t src = apep_text_source_from_string("script.js", src_text);

        apep_note_t notes[2];
        notes[0].kind = "hint";
        notes[0].message = _("hint_semicolons");
        notes[1].kind = "help";
        notes[1].message = _("help_add_semicolon");

        apep_loc_t loc = {2, 23};

        apep_print_text_diagnostic(
            &opt, APEP_SEV_ERROR, "E0042",
            _("err_expected_semicolon"),
            &src, loc, 1, notes, 2);
    }

    /* ================================================== */
    print_separator("sep_4");

    {
        const char *src_text =
            "def greet(name):\n"
            "    message = 'Hello, ' + name\n"
            "    print(message)\n";

        apep_text_source_t src = apep_text_source_from_string("hello.py", src_text);

        apep_note_t notes[1];
        notes[0].kind = "suggestion";
        notes[0].message = _("suggestion_fstrings");

        apep_loc_t loc = {2, 15};

        apep_print_text_diagnostic(
            &opt, APEP_SEV_WARN, "W0102",
            _("warn_string_concat"),
            &src, loc, 15, notes, 1);
    }

    /* ================================================== */
    print_separator("sep_5");

    {
        const char *src_text =
            "{\n"
            "  \"version\": \"1.0\",\n"
            "  \"name\": \"myapp\"\n"
            "}\n";

        apep_text_source_t src = apep_text_source_from_string("package.json", src_text);

        apep_note_t notes[1];
        notes[0].kind = "info";
        notes[0].message = _("info_optional_field");

        apep_loc_t loc = {3, 3};

        apep_print_text_diagnostic(
            &opt, APEP_SEV_NOTE, NULL,
            _("note_missing_description"),
            &src, loc, 0, notes, 1);
    }

    /* ================================================== */
    print_separator("6. BINARY DATA ERROR - Corrupted file header");

    {
        uint8_t header[32] = {
            0x50, 0x4B, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00,
            0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0xAA, 0xBB,
            0xCC, 0xDD, 0x12, 0x34, 0x56, 0x78, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        apep_note_t notes[2];
        notes[0].kind = "expected";
        notes[0].message = "bytes 14-17 should be CRC-32 checksum";
        notes[1].kind = "found";
        notes[1].message = "AA BB CC DD (invalid)";

        apep_span_t span = {14, 4};

        apep_print_hex_diagnostic(
            &opt, APEP_SEV_ERROR, "E_CHECKSUM",
            "invalid CRC-32 in ZIP header",
            "archive.zip", header, sizeof(header),
            span, notes, 2);
    }

    /* ================================================== */
    print_separator("7. PROTOCOL ERROR - Network packet");

    {
        uint8_t packet[48];
        for (size_t i = 0; i < sizeof(packet); i++)
            packet[i] = (uint8_t)(i * 7 % 256);

        apep_note_t notes[1];
        notes[0].kind = "note";
        notes[0].message = "protocol version mismatch detected at offset 0x08";

        apep_span_t span = {8, 2};

        apep_print_hex_diagnostic(
            &opt, APEP_SEV_WARN, "W_PROTO",
            "unsupported protocol version 0x38",
            "packet.raw", packet, sizeof(packet),
            span, notes, 1);
    }

    /* ================================================== */
    print_separator("8. SUGGESTIONS - Show suggested fixes");

    {
        const char *src_text = "int x = \"hello\";";
        apep_text_source_t src = apep_text_source_from_string("main.c", src_text);

        apep_suggestion_t sug = {
            .label = "did you mean?",
            .code = "int x = 42;",
            .loc = {1, 1},
            .replacement_length = 16};

        apep_loc_t loc = {1, 9};
        apep_print_text_diagnostic_with_suggestion(
            &opt, APEP_SEV_ERROR, "E0042", "type mismatch: expected int, got string",
            &src, loc, 7, NULL, 0, &sug);
    }

    /* ================================================== */
    print_separator("9. MULTI-SPAN - Highlight multiple locations");

    {
        const char *src_text = "int x = \"hello\";";
        apep_text_source_t src = apep_text_source_from_string("main.c", src_text);

        apep_text_span_t spans[] = {
            {{1, 1}, 3, "int"},
            {{1, 9}, 7, "expected int, got string"}};

        apep_print_text_diagnostic_multi(
            &opt, APEP_SEV_ERROR, "E0042", "type mismatch",
            &src, spans, 2, NULL, 0);
    }

    /* ================================================== */
    print_separator("10. JSON OUTPUT - Structured diagnostics");

    {
        apep_note_t notes[] = {
            {"hint", "add semicolon at end of statement"},
            {"help", "JavaScript syntax requires ';' or newline"}};

        printf("Colored JSON output:\n");
        apep_print_json_diagnostic(
            stdout, APEP_SEV_ERROR,
            "E0001", "unexpected token",
            "app.js", 42, 15, 1,
            notes, 2);
    }

    /* ================================================== */
    print_separator("11. PERFORMANCE - Measure execution time");

    {
        printf("Timing a simulated operation...\n");
        apep_perf_timer_t *timer = apep_perf_start("file_processing");

        /* Simulate work */
        for (volatile int i = 0; i < 5000000; i++)
            ;

        apep_perf_end(timer, &opt);
    }

    /* ================================================== */
    print_separator("12. PROGRESS - Long-running operations");

    {
        apep_progress_t *prog = apep_progress_start(&opt, "Processing files", 50);

        for (size_t i = 1; i <= 50; i++)
        {
#ifdef _WIN32
            Sleep(10); /* 10ms delay */
#else
            usleep(10000); /* 10ms delay */
#endif
            apep_progress_update(prog, i);
        }

        apep_progress_done(prog);
    }

    /* ================================================== */
    print_separator("13. COLOR SCHEMES - Different palettes");

    {
        const char *src_text = "(1+)";
        apep_text_source_t src = apep_text_source_from_string("test.expr", src_text);
        apep_loc_t loc = {1, 4};

        printf("Scheme: COLORBLIND (accessible)\n");
        apep_set_color_scheme(APEP_SCHEME_COLORBLIND);
        apep_print_text_diagnostic(&opt, APEP_SEV_ERROR, "E001",
                                   "syntax error", &src, loc, 1, NULL, 0);

        printf("\nScheme: DEFAULT (classic)\n");
        apep_set_color_scheme(APEP_SCHEME_DEFAULT);
        apep_print_text_diagnostic(&opt, APEP_SEV_WARN, "W002",
                                   "unused variable", &src, loc, 1, NULL, 0);
    }

    /* ================================================== */
    print_separator("DEMO COMPLETE");

    printf("\n🎨 APEP v%d.%d.%d provides:\n",
           APEP_VERSION_MAJOR, APEP_VERSION_MINOR, APEP_VERSION_PATCH);
    printf("  ✓ Beautiful error messages with context\n");
    printf("  ✓ Structured logging with severity levels\n");
    printf("  ✓ Hexdump diagnostics for binary data\n");
    printf("  ✓ JSON output for IDE/CI integration\n");
    printf("  ✓ Suggestions and multi-span highlighting\n");
    printf("  ✓ Performance metrics and progress bars\n");
    printf("  ✓ Multiple color schemes (incl. colorblind)\n");
    printf("  ✓ Adaptive output (color, Unicode, terminal width)\n");
    printf("  ✓ Zero dependencies, portable C11 code\n");
    printf("\n");
    printf("📚 See docs/ADVANCED.md for all features!\n\n");

    return 0;
}