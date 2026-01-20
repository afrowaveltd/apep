# APEP Advanced Features Guide

This guide covers advanced APEP capabilities for experienced developers.

## Table of Contents

1. [Custom Color Schemes](#custom-color-schemes)
2. [Performance Optimization](#performance-optimization)
3. [Memory Management](#memory-management)
4. [Filter System](#filter-system)
5. [JSON Output](#json-output)
6. [Multi-Span Errors](#multi-span-errors)
7. [Stack Traces](#stack-traces)
8. [Progress Reporting](#progress-reporting)
9. [Capability Detection](#capability-detection)
10. [Buffer Management](#buffer-management)

## Custom Color Schemes

Create custom color schemes programmatically:

```c
#include <apep/apep.h>

void setup_custom_theme(apep_context_t *ctx) {
    apep_scheme_t scheme = {
        .error_color = "\033[38;5;196m",      // Bright red
        .warning_color = "\033[38;5;208m",    // Orange
        .note_color = "\033[38;5;39m",        // Cyan
        .line_number_color = "\033[38;5;240m", // Gray
        .reset = "\033[0m"
    };
    
    apep_set_scheme(ctx, &scheme);
}
```

### Using Built-in Themes

```c
// Apply preset themes
apep_set_theme(ctx, APEP_THEME_MONOKAI);
apep_set_theme(ctx, APEP_THEME_SOLARIZED);
apep_set_theme(ctx, APEP_THEME_GRUVBOX);
```

## Performance Optimization

### Batch Operations

Process multiple errors efficiently:

```c
void report_multiple_errors(apep_context_t *ctx, error_list_t *errors) {
    // Disable auto-flush
    apep_set_auto_flush(ctx, 0);
    
    for (size_t i = 0; i < errors->count; i++) {
        apep_text_error(ctx, errors->items[i].message,
                       errors->items[i].line, errors->items[i].col);
    }
    
    // Flush once at end
    apep_flush(ctx);
}
```

### Buffer Pre-allocation

```c
void optimize_large_output(apep_context_t *ctx) {
    // Pre-allocate 64KB buffer
    apep_buffer_reserve(ctx, 65536);
    
    // Generate large diagnostic
    apep_hex_error(ctx, large_data, large_size, offset);
}
```

## Memory Management

### Zero-Copy Operations

```c
void zero_copy_error(apep_context_t *ctx, const char *source, size_t len) {
    // Use source buffer directly (no copy)
    apep_text_error_nocopy(ctx, source, len, line, col);
}
```

### Arena Allocator

```c
void use_arena(apep_context_t *ctx) {
    // Use arena for temp allocations
    apep_arena_t *arena = apep_arena_create(8192);
    apep_set_allocator(ctx, arena);
    
    // ... generate errors ...
    
    // Free all at once
    apep_arena_destroy(arena);
}
```

## Filter System

Filter diagnostics by severity:

```c
void setup_filters(apep_context_t *ctx) {
    // Only show errors and warnings
    apep_filter_t filter = {
        .show_error = 1,
        .show_warning = 1,
        .show_note = 0,
        .show_help = 0
    };
    
    apep_set_filter(ctx, &filter);
}
```

### Regex Filtering

```c
void regex_filter(apep_context_t *ctx) {
    // Filter by message pattern
    apep_filter_pattern(ctx, "^warning.*deprecated");
}
```

## JSON Output

Export diagnostics as JSON:

```c
#include <apep/apep.h>

void export_json(apep_context_t *ctx) {
    apep_json_t *json = apep_json_create();
    
    // Add error
    apep_text_error(ctx, "Syntax error", 10, 5);
    
    // Export to JSON
    char *json_str = apep_to_json(ctx, json);
    printf("%s\n", json_str);
    
    free(json_str);
    apep_json_destroy(json);
}
```

**Output**:
```json
{
  "diagnostics": [
    {
      "severity": "error",
      "message": "Syntax error",
      "location": {
        "line": 10,
        "column": 5
      }
    }
  ]
}
```

## Multi-Span Errors

Highlight multiple code sections:

```c
void multi_span_error(apep_context_t *ctx, const char *source) {
    apep_multispan_t *ms = apep_multispan_create();
    
    // Primary span
    apep_multispan_add_primary(ms, 10, 5, 10, 15, "First issue");
    
    // Secondary span
    apep_multispan_add_secondary(ms, 20, 8, 20, 12, "Related here");
    
    // Render
    apep_multispan_render(ctx, ms, source);
    
    apep_multispan_destroy(ms);
}
```

**Output**:
```
error: conflicting types
  --> file.c:10:5
   |
10 |     int x = "string";
   |     ^^^^^ expected int
...
20 |     float x = 3.14;
   |           ^ previously defined here
```

## Stack Traces

Generate stack traces:

```c
#include <apep/apep.h>

void show_stack_trace(apep_context_t *ctx) {
    apep_stack_t *stack = apep_stack_capture();
    
    apep_stack_print(ctx, stack, 
                    "Stack trace for error:");
    
    apep_stack_destroy(stack);
}
```

**Output**:
```
Stack trace for error:
  #0: function_c() at file.c:42
  #1: function_b() at file.c:30
  #2: function_a() at file.c:15
  #3: main() at file.c:5
```

## Progress Reporting

Show progress bars:

```c
void long_operation(apep_context_t *ctx) {
    apep_progress_t *prog = apep_progress_create("Processing", 100);
    
    for (int i = 0; i < 100; i++) {
        // Do work
        process_item(i);
        
        // Update progress
        apep_progress_update(prog, i + 1);
    }
    
    apep_progress_finish(prog, "Done!");
    apep_progress_destroy(prog);
}
```

**Output**:
```
Processing: [████████████████----] 80% (80/100)
```

## Capability Detection

Detect terminal capabilities:

```c
void adaptive_output(apep_context_t *ctx) {
    apep_caps_t caps = apep_get_capabilities();
    
    if (caps.colors_256) {
        // Use 256-color palette
        apep_set_theme(ctx, APEP_THEME_GRUVBOX);
    } else if (caps.colors_16) {
        // Use basic 16 colors
        apep_set_theme(ctx, APEP_THEME_BASIC);
    } else {
        // ASCII only
        apep_set_plain(ctx, 1);
    }
    
    if (caps.unicode) {
        // Use Unicode box-drawing
        apep_set_unicode(ctx, 1);
    }
}
```

## Buffer Management

Direct buffer access for advanced use:

```c
void direct_buffer_access(apep_context_t *ctx) {
    apep_buffer_t *buf = apep_get_buffer(ctx);
    
    // Write directly
    apep_buffer_append(buf, "Custom output\n", 14);
    
    // Get raw data
    const char *data = apep_buffer_data(buf);
    size_t size = apep_buffer_size(buf);
    
    // Write to file
    fwrite(data, 1, size, file);
}
```

## Combining Features

Example combining multiple advanced features:

```c
void advanced_diagnostics(apep_context_t *ctx, const char *source) {
    // 1. Setup custom theme
    apep_set_theme(ctx, APEP_THEME_MONOKAI);
    
    // 2. Enable capability detection
    apep_caps_t caps = apep_get_capabilities();
    if (!caps.colors_256) {
        apep_set_plain(ctx, 1);
    }
    
    // 3. Pre-allocate buffer
    apep_buffer_reserve(ctx, 32768);
    
    // 4. Create multi-span error
    apep_multispan_t *ms = apep_multispan_create();
    apep_multispan_add_primary(ms, 10, 5, 10, 15, "Type mismatch");
    apep_multispan_add_secondary(ms, 15, 8, 15, 12, "Expected type");
    
    // 5. Render with stack trace
    apep_multispan_render(ctx, ms, source);
    apep_stack_t *stack = apep_stack_capture();
    apep_stack_print(ctx, stack, NULL);
    
    // 6. Export to JSON
    char *json = apep_to_json(ctx, NULL);
    save_to_file("diagnostics.json", json);
    
    // 7. Cleanup
    apep_multispan_destroy(ms);
    apep_stack_destroy(stack);
    free(json);
}
```

## Best Practices

1. **Performance**: Pre-allocate buffers for large outputs
2. **Memory**: Use arenas for temp allocations
3. **Portability**: Always detect capabilities
4. **Readability**: Use multi-span for complex errors
5. **Integration**: Export JSON for tool integration

## See Also

- [API Reference](API.md) - Complete API documentation
- [Implementation Examples](IMPLEMENTATION_EXAMPLES.md) - Real-world patterns
- [Capabilities](CAPABILITIES.md) - Terminal capability details