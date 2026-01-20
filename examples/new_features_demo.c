/*
 * Comprehensive demo of all new APEP features
 */

#include <apep/apep.h>
#include <apep/apep_helpers.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void demo_json_output(void)
{
    printf("\n=== JSON Output Demo ===\n");

    apep_note_t notes[] = {
        {"hint", "expected expression after '+' operator"},
        {"help", "try adding a number: (1+2)"}};

    apep_print_json_diagnostic(
        stdout,
        APEP_SEV_ERROR,
        "E0001",
        "unexpected token ')'",
        "input.expr",
        1, 4, 1,
        notes, 2);
}

void demo_severity_filter(void)
{
    printf("\n=== Severity Filtering Demo ===\n");

    printf("Setting minimum severity to ERROR (warnings/notes suppressed)\n");
    apep_set_min_severity(APEP_SEV_ERROR);

    printf("Current min severity: %s\n", apep_severity_name(apep_get_min_severity()));

    if (apep_severity_passes_filter(APEP_SEV_ERROR))
        printf("ERROR passes filter ✓\n");
    if (!apep_severity_passes_filter(APEP_SEV_WARN))
        printf("WARN blocked by filter ✗\n");
    if (!apep_severity_passes_filter(APEP_SEV_NOTE))
        printf("NOTE blocked by filter ✗\n");

    /* Reset for other demos */
    apep_set_min_severity(APEP_SEV_NOTE);
}

void demo_buffering(void)
{
    printf("\n=== Diagnostic Buffering Demo ===\n");

    apep_diagnostic_buffer_t *buf = apep_buffer_create();

    printf("Adding diagnostics to buffer...\n");
    apep_buffer_add(buf, APEP_SEV_ERROR, "E001", "undefined variable 'x'", "test.c", 10, 5);
    apep_buffer_add(buf, APEP_SEV_WARN, "W002", "unused parameter 'argc'", "test.c", 5, 15);
    apep_buffer_add(buf, APEP_SEV_ERROR, "E003", "type mismatch", "test.c", 12, 10);
    apep_buffer_add(buf, APEP_SEV_NOTE, "N001", "declared here", "test.c", 3, 8);

    printf("Buffer contains %lu diagnostics\n", (unsigned long)apep_buffer_count(buf));

    printf("\nFlushing buffer (sorted by location):\n");
    apep_buffer_flush(buf, NULL, 1);

    apep_buffer_destroy(buf);
}

void demo_color_schemes(void)
{
    printf("\n=== Color Scheme Demo ===\n");

    const char *source = "(1+)";
    apep_text_source_t src = apep_text_source_from_string("test.expr", source);

    const char *schemes[] = {"DEFAULT", "DARK", "LIGHT", "COLORBLIND"};

    for (int i = 0; i <= 3; i++)
    {
        printf("\n--- Scheme: %s ---\n", schemes[i]);
        apep_set_color_scheme(i);

        apep_options_t opt;
        apep_options_default(&opt);

        apep_loc_t loc = {1, 4};
        apep_print_text_diagnostic(&opt, APEP_SEV_ERROR, "E0001",
                                   "unexpected token", &src, loc, 1, NULL, 0);
    }

    /* Reset to default */
    apep_set_color_scheme(APEP_SCHEME_DEFAULT);
}

void demo_stack_trace(void)
{
    printf("\n=== Stack Trace Demo ===\n");

    APEP_TRACE_BEGIN();
    printf("Pushed frame 1\n");

    APEP_TRACE_BEGIN();
    printf("Pushed frame 2\n");

    APEP_TRACE_BEGIN();
    printf("Pushed frame 3\n");

    printf("\nCurrent stack:\n");
    apep_stack_print(NULL);

    APEP_TRACE_END();
    APEP_TRACE_END();
    APEP_TRACE_END();

    apep_stack_clear();
}

void demo_suggestions(void)
{
    printf("\n=== Suggestions/Diff Demo ===\n");

    const char *source = "int x = \"hello\";";
    apep_text_source_t src = apep_text_source_from_string("test.c", source);
    apep_options_t opt;
    apep_options_default(&opt);

    apep_loc_t loc = {1, 9};
    apep_suggestion_t sug = {
        .label = "did you mean?",
        .code = "int x = 42;",
        .loc = {1, 1},
        .replacement_length = 16};

    apep_print_text_diagnostic_with_suggestion(
        &opt, APEP_SEV_ERROR, "E0042", "type mismatch",
        &src, loc, 7, NULL, 0, &sug);
}

void demo_multi_span(void)
{
    printf("\n=== Multi-Span Highlighting Demo ===\n");

    const char *source = "int x = \"hello\";";
    apep_text_source_t src = apep_text_source_from_string("test.c", source);
    apep_options_t opt;
    apep_options_default(&opt);

    apep_text_span_t spans[] = {
        {{1, 1}, 3, "int"},
        {{1, 9}, 7, "expected int, got string"}};

    apep_print_text_diagnostic_multi(
        &opt, APEP_SEV_ERROR, "E0042", "type mismatch",
        &src, spans, 2, NULL, 0);
}

void demo_performance(void)
{
    printf("\n=== Performance Metrics Demo ===\n");

    apep_perf_timer_t *timer = apep_perf_start("demo_operation");

    /* Simulate work */
    printf("Doing some work...\n");
    for (volatile int i = 0; i < 10000000; i++)
        ;

    apep_perf_end(timer, NULL);
}

void demo_progress(void)
{
    printf("\n=== Progress Reporting Demo ===\n");

    apep_progress_t *prog = apep_progress_start(NULL, "Processing files", 100);

    for (size_t i = 1; i <= 100; i++)
    {
#ifdef _WIN32
        Sleep(20); /* 20ms delay */
#else
        usleep(20000); /* 20ms delay */
#endif
        apep_progress_update(prog, i);
    }

    apep_progress_done(prog);
}

void demo_assertions(void)
{
    printf("\n=== Assertion Macros Demo ===\n");

    int x = 42;
    APEP_ASSERT(x == 42, "x should be 42");
    printf("Assertion passed ✓\n");

    printf("\nNote: The following assertion will fail and abort:\n");
    printf("Uncomment to test: APEP_ASSERT(x == 0, \"x should be 0\");\n");
    /* Uncomment to see assertion failure:
    APEP_ASSERT(x == 0, "x should be 0");
    */
}

int main(void)
{
    printf("╔═══════════════════════════════════════════╗\n");
    printf("║  APEP New Features - Comprehensive Demo  ║\n");
    printf("╚═══════════════════════════════════════════╝\n");

    demo_json_output();
    demo_severity_filter();
    demo_buffering();
    demo_color_schemes();
    demo_stack_trace();
    demo_suggestions();
    demo_multi_span();
    demo_performance();
    demo_progress();
    demo_assertions();

    printf("\n✨ All demos completed successfully!\n\n");
    return 0;
}
