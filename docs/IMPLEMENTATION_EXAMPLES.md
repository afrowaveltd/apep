# APEP Implementation Examples

Real-world patterns for integrating APEP into production applications.

## Table of Contents

1. [Compiler Integration](#1-compiler-integration)
2. [Parser Error Handling](#2-parser-error-handling)
3. [Network Protocol Errors](#3-network-protocol-errors)
4. [Database Query Errors](#4-database-query-errors)
5. [Configuration File Errors](#5-configuration-file-errors)
6. [Build System Integration](#6-build-system-integration)

## 1. Compiler Integration

Integrate APEP into a C compiler for Rust-quality diagnostics.

### Example: Syntax Error in Compiler

```c
#include <apep/apep.h>
#include "compiler.h"

typedef struct {
    apep_context_t *diagnostics;
    const char *filename;
    const char *source;
} compiler_context_t;

void report_syntax_error(compiler_context_t *ctx, 
                         token_t *tok, 
                         const char *expected) {
    // Create diagnostic
    apep_text_error(ctx->diagnostics,
                   "Syntax error",
                   tok->line,
                   tok->column);
    
    // Add context line
    apep_add_line(ctx->diagnostics, ctx->source, tok->line);
    
    // Highlight error position
    apep_add_span(ctx->diagnostics, 
                 tok->column, 
                 tok->length,
                 APEP_SPAN_ERROR);
    
    // Add suggestion
    apep_add_note(ctx->diagnostics,
                 "Expected '%s', found '%s'",
                 expected, tok->text);
    
    // Add help
    apep_add_help(ctx->diagnostics,
                 "Try: %s %s", expected, tok->text);
}

int compile_file(const char *filename) {
    // Setup compiler
    compiler_context_t ctx = {
        .diagnostics = apep_create(),
        .filename = filename,
        .source = read_file(filename)
    };
    
    apep_set_source_file(ctx.diagnostics, filename);
    
    // Parse
    parser_t *parser = parser_create(ctx.source);
    
    while (!parser_eof(parser)) {
        token_t *tok = parser_next(parser);
        
        if (tok->type == TOK_ERROR) {
            report_syntax_error(&ctx, tok, 
                              parser_expected(parser));
        }
    }
    
    // Cleanup
    apep_destroy(ctx.diagnostics);
    parser_destroy(parser);
    free(ctx.source);
    
    return 0;
}
```

**Output**:
```
error: Syntax error
  --> example.c:15:9
   |
15 |     int x = ;
   |             ^ Expected identifier
   |
note: Expected 'identifier', found ';'
help: Try: identifier ;
```

## 2. Parser Error Handling

JSON parser with beautiful error messages.

```c
#include <apep/apep.h>
#include <json-c/json.h>

typedef struct {
    apep_context_t *ctx;
    const char *json_text;
    size_t length;
} json_parser_t;

void report_json_error(json_parser_t *parser, 
                      size_t offset, 
                      const char *message) {
    // Calculate line/column from offset
    size_t line = 1, col = 1;
    for (size_t i = 0; i < offset; i++) {
        if (parser->json_text[i] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
    }
    
    // Report error
    apep_text_error(parser->ctx, message, line, col);
    apep_add_line(parser->ctx, parser->json_text, line);
    apep_add_span(parser->ctx, col, 1, APEP_SPAN_ERROR);
}

json_object* parse_json_safe(const char *json_text) {
    json_parser_t parser = {
        .ctx = apep_create(),
        .json_text = json_text,
        .length = strlen(json_text)
    };
    
    enum json_tokener_error jerr;
    json_object *jobj = json_tokener_parse_verbose(json_text, &jerr);
    
    if (!jobj) {
        // Find error offset
        size_t offset = /* calculate from jerr */;
        
        switch (jerr) {
        case json_tokener_error_parse_unexpected:
            report_json_error(&parser, offset, 
                            "Unexpected token");
            break;
        case json_tokener_error_parse_eof:
            report_json_error(&parser, offset, 
                            "Unexpected end of input");
            break;
        default:
            report_json_error(&parser, offset, 
                            json_tokener_error_desc(jerr));
        }
    }
    
    apep_destroy(parser.ctx);
    return jobj;
}
```

**Output**:
```
error: Unexpected token
  --> config.json:5:12
   |
 5 |     "port": 8080,
   |            ^ Expected string, found number
```

## 3. Network Protocol Errors

HTTP server with protocol error diagnostics.

```c
#include <apep/apep.h>
#include "http_server.h"

void report_http_error(apep_context_t *ctx,
                      const char *request,
                      size_t error_offset,
                      const char *message) {
    // Find line in request
    size_t line = 1;
    for (size_t i = 0; i < error_offset; i++) {
        if (request[i] == '\n') line++;
    }
    
    // Report as hex dump for binary data
    apep_hex_error(ctx, message,
                  (const unsigned char*)request,
                  strlen(request),
                  error_offset);
}

int handle_request(const char *request) {
    apep_context_t *ctx = apep_create();
    
    // Parse HTTP request
    if (!starts_with(request, "GET") &&
        !starts_with(request, "POST")) {
        
        report_http_error(ctx, request, 0,
                         "Invalid HTTP method");
        apep_destroy(ctx);
        return -1;
    }
    
    // Validate headers
    const char *content_length = find_header(request, "Content-Length");
    if (content_length) {
        int len = atoi(content_length);
        if (len < 0) {
            size_t offset = strstr(request, content_length) - request;
            report_http_error(ctx, request, offset,
                            "Negative content length");
            apep_destroy(ctx);
            return -1;
        }
    }
    
    apep_destroy(ctx);
    return 0;
}
```

**Output**:
```
error: Invalid HTTP method
  --> request:1
   |
Hex dump at offset 0:
00000000: 44 45 4c 45 54 45 20 2f  DELET E /
00000008: 20 48 54 54 50 2f 31 2e   HTTP/1.
          ^^^^^^^^
          Invalid HTTP method
```

## 4. Database Query Errors

SQL query validator with position-aware errors.

```c
#include <apep/apep.h>
#include <sqlite3.h>

typedef struct {
    apep_context_t *ctx;
    const char *query;
    sqlite3 *db;
} sql_context_t;

void report_sql_error(sql_context_t *ctx, int offset) {
    const char *errmsg = sqlite3_errmsg(ctx->db);
    
    // Calculate line/col
    size_t line = 1, col = 1;
    for (int i = 0; i < offset; i++) {
        if (ctx->query[i] == '\n') {
            line++;
            col = 1;
        } else {
            col++;
        }
    }
    
    // Report error
    apep_text_error(ctx->ctx, errmsg, line, col);
    apep_add_line(ctx->ctx, ctx->query, line);
    
    // Try to find error token
    const char *token_start = &ctx->query[offset];
    size_t token_len = strcspn(token_start, " \t\n;");
    
    apep_add_span(ctx->ctx, col, token_len, APEP_SPAN_ERROR);
    
    // Add suggestion
    if (strstr(errmsg, "no such table")) {
        apep_add_help(ctx->ctx, "Check table name or CREATE TABLE first");
    }
}

int execute_query_safe(sqlite3 *db, const char *query) {
    sql_context_t ctx = {
        .ctx = apep_create(),
        .query = query,
        .db = db
    };
    
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        int offset = sqlite3_error_offset(db);
        if (offset < 0) offset = 0;
        
        report_sql_error(&ctx, offset);
        
        apep_destroy(ctx.ctx);
        return rc;
    }
    
    // Execute
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    apep_destroy(ctx.ctx);
    
    return rc;
}
```

**Output**:
```
error: no such table: userss
  --> query.sql:3:14
   |
 3 | SELECT * FROM userss WHERE id = 1;
   |              ^^^^^^^ Table not found
   |
help: Check table name or CREATE TABLE first
```

## 5. Configuration File Errors

INI file parser with helpful error messages.

```c
#include <apep/apep.h>
#include "ini_parser.h"

typedef struct {
    apep_context_t *ctx;
    const char *filename;
    const char *content;
} config_parser_t;

void report_config_error(config_parser_t *parser,
                        size_t line_num,
                        const char *line,
                        const char *message) {
    apep_text_error(parser->ctx, message, line_num, 1);
    apep_add_line(parser->ctx, parser->content, line_num);
    
    // Highlight entire line
    apep_add_span(parser->ctx, 1, strlen(line), APEP_SPAN_ERROR);
    
    // Add suggestion based on error
    if (strstr(message, "Invalid section")) {
        apep_add_help(parser->ctx, 
                     "Section must be like: [section_name]");
    } else if (strstr(message, "Invalid key-value")) {
        apep_add_help(parser->ctx,
                     "Format: key = value");
    }
}

int parse_config(const char *filename) {
    config_parser_t parser = {
        .ctx = apep_create(),
        .filename = filename,
        .content = read_file(filename)
    };
    
    apep_set_source_file(parser.ctx, filename);
    
    // Parse line by line
    const char *line = parser.content;
    size_t line_num = 1;
    
    while (*line) {
        const char *line_end = strchr(line, '\n');
        if (!line_end) line_end = line + strlen(line);
        
        size_t line_len = line_end - line;
        char line_buf[256];
        strncpy(line_buf, line, line_len);
        line_buf[line_len] = '\0';
        
        // Skip empty lines and comments
        if (line_buf[0] != '\0' && line_buf[0] != '#') {
            if (line_buf[0] == '[') {
                // Section header
                if (line_buf[line_len - 1] != ']') {
                    report_config_error(&parser, line_num, line_buf,
                                      "Invalid section header");
                }
            } else if (!strchr(line_buf, '=')) {
                // Key-value pair
                report_config_error(&parser, line_num, line_buf,
                                  "Invalid key-value pair");
            }
        }
        
        line = line_end;
        if (*line == '\n') line++;
        line_num++;
    }
    
    apep_destroy(parser.ctx);
    free(parser.content);
    
    return 0;
}
```

**Output**:
```
error: Invalid section header
  --> config.ini:8:1
   |
 8 | [database
   | ^^^^^^^^^^ Missing closing bracket
   |
help: Section must be like: [section_name]
```

## 6. Build System Integration

Makefile/CMake integration for build errors.

```c
#include <apep/apep.h>
#include <sys/wait.h>

typedef struct {
    apep_context_t *ctx;
    const char *build_log;
} build_context_t;

void report_build_error(build_context_t *ctx,
                       const char *file,
                       int line,
                       const char *message) {
    apep_text_error(ctx->ctx, message, line, 1);
    
    // Read source file
    char *source = read_file(file);
    if (source) {
        apep_add_line(ctx->ctx, source, line);
        free(source);
    }
    
    // Suggest fix based on error
    if (strstr(message, "undeclared")) {
        apep_add_help(ctx->ctx, 
                     "Add #include or declare variable");
    } else if (strstr(message, "expected")) {
        apep_add_help(ctx->ctx,
                     "Check syntax");
    }
}

int build_project(void) {
    build_context_t ctx = {
        .ctx = apep_create(),
        .build_log = NULL
    };
    
    // Run make
    FILE *make_output = popen("make 2>&1", "r");
    char line[1024];
    
    while (fgets(line, sizeof(line), make_output)) {
        // Parse GCC error format: file.c:line:col: error: message
        char file[256];
        int line_num, col;
        char message[512];
        
        if (sscanf(line, "%[^:]:%d:%d: error: %[^\n]",
                  file, &line_num, &col, message) == 4) {
            report_build_error(&ctx, file, line_num, message);
        }
    }
    
    int status = pclose(make_output);
    apep_destroy(ctx.ctx);
    
    return WEXITSTATUS(status);
}
```

**Output**:
```
error: 'x' undeclared (first use in this function)
  --> main.c:15:5
   |
15 |     x = 42;
   |     ^ Variable not declared
   |
help: Add #include or declare variable
```

## Best Practices

1. **Context**: Always provide source file and line number
2. **Spans**: Highlight exact error location
3. **Suggestions**: Add helpful notes and fixes
4. **Consistency**: Use same error format across project
5. **Colors**: Detect terminal capabilities

## See Also

- [Advanced Features](ADVANCED.md) - Advanced APEP capabilities
- [API Reference](API.md) - Complete API documentation
- [Examples](../examples/README.md) - Demo programs