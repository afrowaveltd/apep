# APEP Locales

Translation files for APEP error messages.

## Available Languages

- `en.loc` - English (default)
- `cs.loc` - Czech (čeština)

## File Format

Each `.loc` file contains key-value pairs:

```
key=value
# Comment
another_key=another value
```

## Adding a New Language

1. Copy `en.loc` to `<lang>.loc` (e.g., `de.loc` for German)
2. Translate values (keep keys unchanged)
3. Test: `./bin/i18n_demo --lang <lang>`
4. Submit PR

## Translation Guidelines

- Keep formatting codes: `%s`, `%d`, etc.
- Preserve ANSI color codes
- Test with demo programs
- Check for encoding (UTF-8)

See [docs/I18N.md](../docs/I18N.md) for details.