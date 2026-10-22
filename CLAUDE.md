# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A traditional Synergy DBL demo of SQL Connection: the same programs run on the
legacy SQL Server driver (Trifox `VTX12_SQLNATIVE` over ODBC and a DSN) or on
the new Synergex driver, switched by one line in `orderdesk.cfg`. Windows x64
only for the UI app; SQL Server is reached over the network. `README.md` has
the database, DSN and configuration setup.

## Commands

Run from the repo root in a Synergy/DE x64 command prompt. There are no tests
or linters; a clean compile and link is the check.

- `build` (`build.bat`): compiles `src\` into `bin\` and links `orderdesk` with
  `bin\speedgraph` (compiled separately) and `WND:tklib.elb`. It sets the `ODSRC` logical to `src\`, which every
  `.include "ODSRC:*.inc"` relies on; set `ODSRC` yourself if you call
  `dbl`/`dblink` directly (`dbl -o bin\<name> src\<name>`, then
  `dblink -o bin\<name>.dbr bin\<name> [WND:tklib.elb]`).
- `run` (`run.bat`): starts the UI app with `dbr bin\orderdesk` and
  `SYN_RESIZE_SCALE=1`. UI Toolkit does not run under `dbs`; on Windows, `dbr`
  output from console programs opens a window that closes at exit.
- `dbs bin\bench` / `dbs bin\errors`: the console programs.
- `powershell -ExecutionPolicy Bypass -File scripts\reset-demo.ps1`: warms up
  both drivers with `bench`, runs `sql\reset.sql`, makes the legacy line active.
- `sql\setup.sql` rebuilds the `OrderDesk` database from scratch (drops it
  first); it needs SQLCMD mode for `$(DemoPassword)`.

Programs read `orderdesk.cfg` and load `assets\` relative to the current
folder, so they must run from the repo root. Files use CRLF (`.gitattributes`).

## Architecture

**Driver switch.** `src\config.inc` (included by all three programs) takes the
first line starting `SQL_CONNECT=` in `orderdesk.cfg`; lines starting `;` are
commented out. The text before the first `:` is shown as the driver. The DBL
runtime, not this code, picks the library: on driver-selection builds
`sqlserver:` goes to `rssql.dll` and everything else to Trifox `vtxapits.dll`;
older single-DLL builds use `vtx12_syn:` for the new driver. The new driver's
connect string has no database, so the `orderdesk` login's default database
(set in `setup.sql`) selects `OrderDesk`.

**`src\orderdesk.dbl`** is a UI Toolkit app built entirely in code (menus,
toolbar, input windows, lists; no window scripts). Shared state lives in the
global data section `odstate`, declared in every routine through
`odstate.inc` (`main` declares it with `init`), so changing its layout means
editing only `odstate.inc`. Lists go through one generic pair: callers fill
the `lines` global section, `show_lines` displays it, `line_load` is the load
method. `main` loops on `M_PROCESS`; toolbar buttons call `toolbar_method`,
which re-signals the same menu entry names. The home panel is a borderless
window sized from the Toolkit body (`g_first_row`/`g_last_row`/
`g_screen_width` in `WND:gblctl.def`) and is redrawn by `home_refresh` on every
loop.

**Timed steps** (`nightly_report`, `post_batch`, and the console `bench.dbl`)
count network round trips with `sys.dm_exec_connections.num_reads` for the
session, which needs `VIEW SERVER STATE` (granted in `setup.sql`). OrderDesk
reads the counter twice before timing to measure one reading's cost
(`per_query`), shows live readings during the loop, and subtracts
`queries * per_query` from the total. The progress window also hosts the speed graph from `speedgraph.dbl`, a separately
compiled unit with its own `graph` global data section (`speedgraph.inc`):
`graph_start` sizes it and returns how many columns fit, the caller samples
every `total / columns` rows, and `graph_add` appends each interval's speed.
The initial scale is `best_rate` (fastest recorded run of that step, either
driver, plus a quarter), so both drivers draw on the same scale; a faster
sample raises it and redraws. Each run is inserted into `demo_runs`;
`verdict`, the home scoreboard and Reports > Compare runs all read the latest
row per driver and step, so two app windows on different drivers see each
other's results. File > Start fresh and `sql\reset.sql` clear `demo_runs` and
orders above seed order 10420.

**Colours.** `theme.inc` names palette entries 11-15, set with `WP_PALET` at
startup (Toolkit uses the lower entries). `accent_for` maps the driver token to
the legacy (amber) or new (green) accent. The three `%ssc_cmd` settings in
`main` are deliberate: the legacy driver accepts them and the new driver
rejects each with "... is not supported; remove this option", which is the
demo's migration step. `main` collects all failures, shows them together,
writes them to `orderdesk.log`, and exits.

## Traditional DBL pitfalls hit in this code

- Plain `record` in a subroutine is static and keeps values between calls.
  `stack record` fields have no initial value; every routine here names its
  stack records and runs `init` on them first. Keep that pattern.
- `.define` names are case-insensitive and stay defined for the rest of the
  file, so they must not collide with any variable or routine name (`TICK` vs
  `tick`, `TB_BUTTON` vs `tb_button`) or with a define in an earlier routine.
- `if (...) then` requires an `else`; substrings of literals are not allowed;
  `.and.`/`.or.` do not short-circuit.
- Toolkit: `U_START` closes channels 1-255, so open files afterwards with a
  channel variable set to 0. Menu entry names are at most 10 characters and are
  matched in `USING` with a trailing blank. On Windows the footer is a
  status-bar section that can truncate text, so the app uses no footer.
- The 64-bit runtime cannot load 32-bit ActiveX controls (such as the Toolkit
  example `Progress.ocx`); the progress bar is drawn with reverse-video text.
