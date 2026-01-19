#include "../include/apep/apep.h"
#include "../include/apep/apep_helpers.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static int streq(const char *a, const char *b)
{
    return a && b && strcmp(a, b) == 0;
}

static void usage(void)
{
    puts("APEP (Afrowave Pretty Error Print) - Demonstration");
    puts("");
    puts("Usage: apep_show_demo [OPTIONS]");
    puts("");
    puts("Options:");
    puts("  --no-color    Disable colored output");
    puts("  --ascii       Use ASCII-only characters (no Unicode)");
    puts("  --plain       Equivalent to --no-color --ascii");
    puts("  --help, -h    Show this help message");
    puts("");
}

static void print_separator(const char *title)
{
    printf("\n");
    printf("==================================================\n");
    printf("  %s\n", title);
    printf("==================================================\n");
    printf("\n");
}

int main(int argc, char **argv)
{
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
            return 0;
        }
        else
        {
            fprintf(stderr, "Unknown option: %s\n\n", argv[i]);
            usage();
            return 2;
        }
    }

    printf("APEP v%d.%d.%d - Afrowave Pretty Error Print\n",
           APEP_VERSION_MAJOR, APEP_VERSION_MINOR, APEP_VERSION_PATCH);

    /* ================================================== */
    print_separator("1. LOG MESSAGES - Various severity levels");

    apep_print_message(&opt, APEP_LVL_TRACE, "BOOT", "initializing subsystems");
    apep_print_message(&opt, APEP_LVL_DEBUG, "CFG", "loaded config from /etc/app.conf");
    apep_print_message(&opt, APEP_LVL_INFO, "NET", "listening on 0.0.0.0:8080");
    apep_print_message(&opt, APEP_LVL_WARN, "DB", "connection pool nearly exhausted (8/10 used)");
    apep_print_message(&opt, APEP_LVL_ERROR, "AUTH", "invalid credentials for user 'admin'");
    apep_print_message(&opt, APEP_LVL_CRITICAL, "SYS", "failed to allocate memory - terminating");

    /* ================================================== */
    print_separator("2. SYNTAX ERROR - Single character issue");

    {
        const char *src_text = "(1+)\n";
        apep_text_source_t src = apep_text_source_from_string("input.expr", src_text);

        apep_note_t notes[1];
        notes[0].kind = "hint";
        notes[0].message = "expected expression after '+' operator";

        apep_loc_t loc = {1, 4};

        apep_print_text_diagnostic(
            &opt, APEP_SEV_ERROR, "E0001",
            "unexpected token ')'",
            &src, loc, 1, notes, 1);
    }

    /* ================================================== */
    print_separator("3. PARSER ERROR - Multi-line context");

    {
        const char *src_text =
            "function calculate(x, y) {\n"
            "    let result = x + y\n"
            "    return result\n"
            "}\n";

        apep_text_source_t src = apep_text_source_from_string("script.js", src_text);

        apep_note_t notes[2];
        notes[0].kind = "hint";
        notes[0].message = "JavaScript requires semicolons or newlines";
        notes[1].kind = "help";
        notes[1].message = "add ';' after the expression";

        apep_loc_t loc = {2, 23};

        apep_print_text_diagnostic(
            &opt, APEP_SEV_ERROR, "E0042",
            "expected ';' at end of statement",
            &src, loc, 1, notes, 2);
    }

    /* ================================================== */
    print_separator("4. TYPE ERROR - Warning with suggestion");

    {
        const char *src_text =
            "def greet(name):\n"
            "    message = 'Hello, ' + name\n"
            "    print(message)\n";

        apep_text_source_t src = apep_text_source_from_string("hello.py", src_text);

        apep_note_t notes[1];
        notes[0].kind = "suggestion";
        notes[0].message = "consider using f-strings: f'Hello, {name}'";

        apep_loc_t loc = {2, 15};

        apep_print_text_diagnostic(
            &opt, APEP_SEV_WARN, "W0102",
            "string concatenation in loop may be inefficient",
            &src, loc, 15, notes, 1);
    }

    /* ================================================== */
    print_separator("5. VALIDATION NOTE - Informational");

    {
        const char *src_text =
            "{\n"
            "  \"version\": \"1.0\",\n"
            "  \"name\": \"myapp\"\n"
            "}\n";

        apep_text_source_t src = apep_text_source_from_string("package.json", src_text);

        apep_note_t notes[1];
        notes[0].kind = "info";
        notes[0].message = "this field is optional but recommended";

        apep_loc_t loc = {3, 3};

        apep_print_text_diagnostic(
            &opt, APEP_SEV_NOTE, NULL,
            "missing 'description' field",
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
            usleep(10000); /* 10ms delay */
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