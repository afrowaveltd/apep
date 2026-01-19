# APEP — Design

APEP (Afrowave Pretty Error Print) is a small C library whose only responsibility is to **render diagnostics for humans**.

It does not decide what is an error.
It does not change program flow.
It does not store state.

APEP only answers one question:

> Given a diagnostic fact, how do we show it truthfully and clearly to a human?

---

## Design goals

1. **Truth first**
   Diagnostics must never guess, infer, or hide uncertainty.

2. **Human readability**
   Output must be understandable without reading documentation.

3. **Graceful degradation**
   When output capabilities are limited, APEP reduces visual features but never loses information.

4. **Portability**
   The library must work across Linux, FreeBSD, Windows, macOS, and low‑resource systems.

5. **Small, stable API**
   The API should be easy to bind, re‑implement, or port to other languages.

---

## Core philosophy

APEP deliberately separates **diagnostic data** from **diagnostic rendering**.

* Host applications decide:

  * what failed
  * where it failed
  * how serious it is

* APEP decides:

  * how to present this information
  * how much visual richness is possible
  * how to degrade output safely

This separation allows APEP to be reused in:

* test engines
* parsers
* CLI tools
* debug modes of production applications

---

## Capabilities model

APEP detects output capabilities at runtime:

* TTY vs non‑TTY output
* color availability
* Unicode line drawing availability
* terminal width

Capabilities are represented as a simple struct and may be:

* auto‑detected
* explicitly overridden by the host application

If a capability is unavailable, APEP falls back to:

* ASCII instead of Unicode
* no color instead of ANSI escapes
* conservative layouts instead of adaptive ones

The output must remain valid, aligned, and readable in all cases.

---

## Diagnostic domains

APEP supports two primary diagnostic domains.

### Text diagnostics

Text diagnostics represent errors tied to human‑readable input:

* source files
* configuration files
* user input
* expressions

Features:

* file or label name
* line and column (1‑based)
* context lines
* caret or span highlight
* notes and hints

### Binary diagnostics

Binary diagnostics represent errors tied to opaque data:

* binary files
* network packets
* serialized formats

Features:

* absolute byte offset
* configurable context window
* hex dump with ASCII preview
* highlighted byte span

Binary data is never printed raw.
Only textual representations are used.

---

## Output behavior

APEP never writes structured reports.
It prints **human‑oriented diagnostics** only.

Structured output (AJIS, JSON, JUnit, etc.) is the responsibility of the host application.

APEP output is intended for:

* stderr
* interactive debugging
* CI logs (with reduced features)

---

## Determinism

When requested by the host application, APEP must produce deterministic output:

* fixed width
* colors disabled
* stable formatting

This is essential for automated testing and diff‑based validation.

---

## Non‑goals

APEP intentionally does not provide:

* logging facilities
* localization
* message catalogs
* file reading beyond minimal helpers
* global configuration or state

These concerns belong to higher‑level systems.

---

## Compatibility and future ports

APEP is designed so that its conceptual model can be re‑implemented in other languages:

* severity
* location or span
* notes
* capabilities

The goal is conceptual compatibility, not byte‑for‑byte identical output.

---

## Stability promise

Once the API reaches version 1.0:

* existing functions will not change semantics
* new features will be additive
* behavior changes will be explicit and documented

---

APEP aims to be a tool developers trust.
When it prints an error, it should feel calm, precise, and honest.
