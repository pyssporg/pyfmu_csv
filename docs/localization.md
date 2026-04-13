# Localization Notes

## Documentation Structure

Documentation is intentionally split by topic instead of keeping one large README:

- `overview.md`
- `architecture.md`
- `usage.md`
- `testing.md`
- `limitations.md`

This keeps each document focused and easier to translate or propagate into other systems.

## Recommended Localization Pattern

When localization starts:

- keep file names and topic boundaries stable
- translate one topic file at a time
- avoid duplicating technical facts across multiple files when a single source can be linked
- keep command examples identical across locales where possible

## Information Propagation

To keep downstream documentation synchronized:

- use `overview.md` for product intent
- use `architecture.md` for design decisions
- use `usage.md` for user-facing command flows
- use `testing.md` for engineering verification steps
- use `limitations.md` for current constraints and non-goals
