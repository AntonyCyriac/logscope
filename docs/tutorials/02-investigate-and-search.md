# Tutorial 2 — Investigate and Search

Filter lines, run structured queries, and use the search command.

## Prerequisites

- `samples/sample.log`

## Filter DSL

```bash
logscope investigate --filter "level == ERROR" samples/sample.log
logscope query --filter "level == WARN" samples/sample.log
```

## Content search

```bash
logscope investigate --search "Connection refused" samples/sample.log
logscope search --query "error AND timeout" samples/sample.log
```

## JSON investigation output

```bash
logscope investigate --format json --filter "level == ERROR" samples/sample.log
```

## Analytics

```bash
logscope analytics samples/sample.log
logscope analytics --format json --bucket 60 samples/sample.log
```

## Sessions

Save and reload investigation context:

```bash
logscope session save my.session samples/sample.log --min-errors 1
logscope session load my.session
```

## Next

[Tutorial 3 — Plugins and AI](03-plugins-and-ai.md)
