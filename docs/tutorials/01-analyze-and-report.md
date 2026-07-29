# Tutorial 1 — Analyze and Report

Walk through analyzing a log file and exporting reports.

## Prerequisites

- LogScope built (`build/apps/cli/logscope` or `logscope.exe`)
- Sample log: `samples/sample.log`

## Analyze (default text report)

```bash
logscope analyze samples/sample.log
```

You should see sections such as summary, log levels, and errors.

## JSON output

```bash
logscope analyze --format json samples/sample.log
```

## HTML and PDF

```bash
logscope analyze --format html --output report.html samples/sample.log
logscope analyze --format pdf --output report.pdf samples/sample.log
```

## Sections and charts

```bash
logscope analyze --sections executive,summary,levels,charts samples/sample.log
```

## Configuration file

```bash
logscope analyze --config samples/logscope.properties samples/sample.log
```

## Persistent index

For large logs, persist the index for faster reopen:

```bash
logscope analyze --persist-index samples/sample.log
logscope investigate --reuse-index --filter "level == ERROR" samples/sample.log
```

## Next

[Tutorial 2 — Investigate and Search](02-investigate-and-search.md)
