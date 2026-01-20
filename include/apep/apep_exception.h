/**
 * @file apep_exception.h
 * @brief Exception handling with stack traces and chaining
 *
 * Provides .NET/Java-style exception handling for C with:
 * - Exception types and messages
 * - Stack trace capture
 * - Exception chaining (inner exceptions)
 * - Formatted output with APEP styling
 */

#ifndef APEP_EXCEPTION_H
#define APEP_EXCEPTION_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* Forward declaration for apep_options_t */
    typedef struct apep_options apep_options_t;

    /**
     * @brief Exception structure
     *
     * Represents a single exception with type, message, source location,
     * stack trace, and optional inner exception (cause).
     */
    typedef struct apep_exception_s
    {
        const char *type;               /**< Exception type (e.g., "NullPointerException") */
        char *message;                  /**< Error message */
        const char *source_file;        /**< Source file where exception was thrown */
        int source_line;                /**< Line number where exception was thrown */
        void *stack_trace;              /**< Stack trace (implementation-dependent) */
        struct apep_exception_s *inner; /**< Inner exception (cause) */
        int error_code;                 /**< Error code (errno or custom) */
        void *user_data;                /**< Optional user data */
        int owns_message;               /**< 1 if message should be freed */
        int owns_inner;                 /**< 1 if inner exception should be freed */
    } apep_exception_t;

    /**
     * @brief Create a new exception
     *
     * @param type Exception type string (not copied, must remain valid)
     * @param format Printf-style format string for message
     * @param ... Format arguments
     * @return New exception instance (caller must free with apep_exception_destroy)
     */
    apep_exception_t *apep_exception_create(const char *type, const char *format, ...);

    /**
     * @brief Set source location for exception
     *
     * @param ex Exception instance
     * @param file Source file name
     * @param line Line number
     */
    void apep_exception_set_source(apep_exception_t *ex, const char *file, int line);

    /**
     * @brief Set inner exception (cause)
     *
     * @param ex Exception instance
     * @param inner Inner exception (will be owned and freed)
     */
    void apep_exception_set_inner(apep_exception_t *ex, apep_exception_t *inner);

    /**
     * @brief Set error code
     *
     * @param ex Exception instance
     * @param code Error code (errno or custom)
     */
    void apep_exception_set_code(apep_exception_t *ex, int code);

    /**
     * @brief Capture current stack trace
     *
     * @param ex Exception instance
     */
    void apep_exception_capture_stack(apep_exception_t *ex);

    /**
     * @brief Print exception with formatting
     *
     * @param opt APEP options (can be NULL for defaults)
     * @param ex Exception to print
     */
    void apep_exception_print(const apep_options_t *opt, const apep_exception_t *ex);

    /**
     * @brief Print exception chain (includes inner exceptions)
     *
     * @param opt APEP options (can be NULL for defaults)
     * @param ex Exception to print
     * @param max_depth Maximum depth to print (0 = unlimited)
     */
    void apep_exception_print_chain(const apep_options_t *opt, const apep_exception_t *ex, int max_depth);

    /**
     * @brief Destroy exception and free resources
     *
     * @param ex Exception to destroy
     */
    void apep_exception_destroy(apep_exception_t *ex);

/**
 * @brief Helper macro to create and print exception
 *
 * Creates exception, sets source location, captures stack,
 * prints it, and destroys it in one call.
 *
 * @param opt APEP options (can be NULL)
 * @param type Exception type
 * @param ... Printf-style format and arguments
 */
#define APEP_EXCEPTION(opt, type, ...)                                    \
    do                                                                    \
    {                                                                     \
        apep_exception_t *_ex = apep_exception_create(type, __VA_ARGS__); \
        apep_exception_set_source(_ex, __FILE__, __LINE__);               \
        apep_exception_capture_stack(_ex);                                \
        apep_exception_print(opt, _ex);                                   \
        apep_exception_destroy(_ex);                                      \
    } while (0)

/**
 * @brief Helper macro to create exception with return
 *
 * Like APEP_EXCEPTION but also returns from current function.
 *
 * @param opt APEP options (can be NULL)
 * @param retval Return value
 * @param type Exception type
 * @param ... Printf-style format and arguments
 */
#define APEP_EXCEPTION_RETURN(opt, retval, type, ...)                     \
    do                                                                    \
    {                                                                     \
        apep_exception_t *_ex = apep_exception_create(type, __VA_ARGS__); \
        apep_exception_set_source(_ex, __FILE__, __LINE__);               \
        apep_exception_capture_stack(_ex);                                \
        apep_exception_print(opt, _ex);                                   \
        apep_exception_destroy(_ex);                                      \
        return (retval);                                                  \
    } while (0)

/**
 * @brief Helper macro to wrap inner exception
 *
 * Creates new exception with inner exception as cause.
 *
 * @param opt APEP options (can be NULL)
 * @param type New exception type
 * @param inner_ex Inner exception
 * @param ... Printf-style format and arguments for new message
 */
#define APEP_EXCEPTION_WRAP(opt, type, inner_ex, ...)                     \
    do                                                                    \
    {                                                                     \
        apep_exception_t *_ex = apep_exception_create(type, __VA_ARGS__); \
        apep_exception_set_source(_ex, __FILE__, __LINE__);               \
        apep_exception_capture_stack(_ex);                                \
        apep_exception_set_inner(_ex, inner_ex);                          \
        apep_exception_print_chain(opt, _ex, 0);                          \
        apep_exception_destroy(_ex);                                      \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* APEP_EXCEPTION_H */
