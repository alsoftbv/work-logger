# Work Logger

A CLI tool for freelancers to log work hours and generate invoices.

## Features

- Log daily work hours per client
- Generate PDF invoices with VAT calculation
- Generate PDF work log reports

## Installation

```bash
cmake -B build
cmake --build build
cp build/app/wlog ~/.local/bin/
```

## Usage

### Initial Setup

Run `wlog` without arguments to set up your business info:

```bash
wlog
```

### Add a Client

```bash
wlog <client-id>
```

### Log Hours

```bash
wlog <client> <hours> "description"
wlog <client> <hours> "description" <date>   # specific date (YYYY-MM-DD)
```

### View Logs

```bash
wlog <client> --show                    # current month
wlog <client> --show --month 1          # specific month (January of current year)
wlog <client> --show --month 2026-01    # specific month (January 2026)
wlog <client> --show --today            # today only
```

### Generate Documents

```bash
wlog <client> --invoice                          # invoice (previous month)
wlog <client> --report                           # work log report (previous month)
wlog <client> --invoice --report --month 2026-01 # both for specific month
```

## Flags

| Flag | Description |
|------|-------------|
| `--show, -s` | Show work logs |
| `--today, -t` | Filter to today only (with --show) |
| `--invoice, -i` | Generate invoice PDF |
| `--report, -r` | Generate work log PDF |
| `--month, -m` | Specify month (YYYY-MM or just month number) |
| `--setup` | Run business setup |

## Building with Tests

```bash
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build
```
