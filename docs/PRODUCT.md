# LogScope

LogScope is a lightweight command-line tool for analyzing application logs.

## Problem

Developers often spend significant time manually searching large log files to identify recurring errors and important events.

## Goal

LogScope analyzes log files and produces a simple summary of:

- Total log lines
- Log severity counts
- Recurring error patterns
- First and last error occurrence
- Console and JSON reports

## Initial Scope

Version 0.1 will support plain-text log files and run as a native C++ command-line application.
