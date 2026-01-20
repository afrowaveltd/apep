/**
 * @file exception_demo.c
 * @brief Demonstrates APEP exception handling
 *
 * Shows:
 * - Simple exceptions
 * - Exception chaining (inner exceptions)
 * - Stack traces
 * - Error codes
 * - Helper macros
 */

#include "apep/apep.h"
#include "apep/apep_exception.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// Simulate deep call stack for stack trace demo
static int level3_fails(const apep_options_t *opt)
{
    APEP_EXCEPTION(opt, "NullPointerException", "Pointer 'data' was NULL");
    return -1;
}

static int level2_calls_level3(const apep_options_t *opt)
{
    return level3_fails(opt);
}

static int level1_calls_level2(const apep_options_t *opt)
{
    return level2_calls_level3(opt);
}

// Demonstrate exception chaining
static apep_exception_t *network_connect_fails(void)
{
    apep_exception_t *ex = apep_exception_create(
        "NetworkException",
        "Connection timeout after 30 seconds");
    apep_exception_set_source(ex, __FILE__, __LINE__);
#ifdef _WIN32
    apep_exception_set_code(ex, 10060); /* WSAETIMEDOUT on Windows */
#else
    apep_exception_set_code(ex, ETIMEDOUT);
#endif
    apep_exception_capture_stack(ex);
    return ex;
}

static int database_connect_fails(const apep_options_t *opt)
{
    apep_exception_t *network_ex = network_connect_fails();

    apep_exception_t *db_ex = apep_exception_create(
        "DatabaseException",
        "Failed to connect to PostgreSQL database 'myapp'");
    apep_exception_set_source(db_ex, __FILE__, __LINE__);
    apep_exception_set_code(db_ex, -1);
    apep_exception_capture_stack(db_ex);
    apep_exception_set_inner(db_ex, network_ex);

    apep_exception_print_chain(opt, db_ex, 0);
    apep_exception_destroy(db_ex);

    return -1;
}

// File operations with exceptions
static int file_operation_fails(const apep_options_t *opt)
{
    FILE *f = fopen("nonexistent_file.txt", "r");
    if (!f)
    {
        APEP_EXCEPTION_RETURN(opt, -1, "FileNotFoundException",
                              "Could not open file 'nonexistent_file.txt'");
    }
    return 0;
}

// Array bounds exception
static void array_bounds_exception(const apep_options_t *opt)
{
    int arr[10];
    (void)arr;
    int index = 15;

    APEP_EXCEPTION(opt, "IndexOutOfBoundsException",
                   "Index %d is out of bounds for array of size %d",
                   index, 10);
}

// Arithmetic exception
static void division_by_zero(const apep_options_t *opt)
{
    int divisor = 0;

    APEP_EXCEPTION(opt, "ArithmeticException",
                   "Division by zero: cannot divide %d by %d",
                   42, divisor);
}

// Invalid argument exception
static int process_data(const apep_options_t *opt, const char *data, size_t len)
{
    if (!data)
    {
        APEP_EXCEPTION_RETURN(opt, -1, "IllegalArgumentException",
                              "Argument 'data' cannot be NULL");
    }

    if (len == 0)
    {
        APEP_EXCEPTION_RETURN(opt, -1, "IllegalArgumentException",
                              "Argument 'len' must be greater than 0, got %zu", len);
    }

    return 0;
}

static void print_separator(const char *title)
{
    printf("\n");
    printf("=== ");
    printf("%s", title);
    printf(" ===\n");
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    apep_options_t opt = {0};
    apep_options_default(&opt);

    printf("APEP Exception Handling Demo\n");
    printf("============================\n\n");

    // 1. Simple exception with stack trace
    print_separator("Simple Exception with Stack Trace");
    level1_calls_level2(&opt);

    // 2. Exception chaining (inner exceptions)
    print_separator("Exception Chaining (Inner Exceptions)");
    database_connect_fails(&opt);

    // 3. File operation exception
    print_separator("File Operation Exception");
    file_operation_fails(&opt);

    // 4. Array bounds exception
    print_separator("Index Out of Bounds");
    array_bounds_exception(&opt);

    // 5. Arithmetic exception
    print_separator("Arithmetic Exception");
    division_by_zero(&opt);

    // 6. Invalid argument exceptions
    print_separator("Invalid Argument Exceptions");
    process_data(&opt, NULL, 10);

    process_data(&opt, "data", 0);

    // 7. Exception wrapping demo
    print_separator("Exception Wrapping");

    // Simulate nested exception scenario
    apep_exception_t *io_ex = apep_exception_create(
        "IOException",
        "Disk full: cannot write to /var/log/app.log");
    apep_exception_set_source(io_ex, __FILE__, __LINE__);
    apep_exception_set_code(io_ex, ENOSPC);
    apep_exception_capture_stack(io_ex);

    apep_exception_t *log_ex = apep_exception_create(
        "LoggingException",
        "Failed to write log entry");
    apep_exception_set_source(log_ex, __FILE__, __LINE__);
    apep_exception_capture_stack(log_ex);
    apep_exception_set_inner(log_ex, io_ex);

    apep_exception_t *app_ex = apep_exception_create(
        "ApplicationException",
        "Critical system error during startup");
    apep_exception_set_source(app_ex, __FILE__, __LINE__);
    apep_exception_capture_stack(app_ex);
    apep_exception_set_inner(app_ex, log_ex);

    apep_exception_print_chain(&opt, app_ex, 0);
    apep_exception_destroy(app_ex);

    printf("\n=== Demo Complete ===\n");
    printf("Exception handling provides:\n");
    printf("  ✓ Type information\n");
    printf("  ✓ Error messages\n");
    printf("  ✓ Source locations\n");
    printf("  ✓ Stack traces\n");
    printf("  ✓ Exception chaining\n");
    printf("  ✓ Error codes\n");

    return 0;
}
