# OrderDesk: SQL Connection demo

Live demo for DevPartner 2026, "Rebuilding SQL Connection from the Ground Up".

- `src\orderdesk.dbl` is a UI Toolkit order-entry app: browse customers, place an
  order, post a batch of 1,000 orders, run a 2,000,000-row nightly report, and
  compare the timed runs of both drivers. It also applies three old connection
  settings at startup that the new driver no longer accepts.
- `src\bench.dbl` is the console version of the timed steps, for clean numbers and
  as a fallback.
- `src\errors.dbl` runs three failing statements and prints what the driver
  reports.

Switching between the legacy SQL Connection driver and the new one is one line
in `orderdesk.cfg`. SQL Server runs in a Hyper-V VM on the same laptop, on the
Default Switch, reached by the name `DESKTOP-NGU3456.mshome.net`. Nothing is
copied into the VM.

What the demo shows, and what not to claim:

- **Drop-in, mostly.** The same `.dbr` runs on both drivers after a one-line
  config change, unless the app uses settings the new driver retired. Then the
  driver names each one ("... is not supported; remove this option"), and
  removing them is a small edit.
- **No DSN, no ODBC.** The new connect string names the server directly.
  Trifox still loads at `%SSC_INIT`.
- **Reads.** Fewer round trips and a shorter time. Measured on Linux: about 11x
  fewer round trips and 2.4x faster. Measure Windows yourself.
- **Writes.** One transaction per order. Measured on Linux: 3 round trips per
  order on legacy, 2 on the new driver, about the same time. Don't claim writes
  are faster.
- **Errors** (`errors.dbl`, for Q&A). Same SQLCODE for ordinary SQL errors with
  the server's message; for a procedure call SQL Server rejects, legacy reports
  `-1` "not found" while the new driver reports the real error.

## Build requirements

A Synergy/DE x64 build with the driver-selection runtime (both drivers load
side by side; `sqlserver:` goes to `rssql.dll`), `project/ssql-rust` (server
error text and the "remove this option" messages), `rssql/perf` (read
speedups), and an optimized Rust build. `dev.bat build` deploys an unoptimized
debug build; never time it. On a build without driver selection, the new line
starts with `vtx12_syn:`, only one driver loads at a time, and
`scripts\reset-demo.ps1` cannot warm up both.

## Part 1: Database

1. **VM, SSMS (Windows authentication):** server **Properties > Security** >
   **SQL Server and Windows Authentication mode**, then **Restart**. Once.
2. **VM, SSMS:** **New Query**, paste `sql\setup.sql`, turn on **Query > SQLCMD
   Mode**, add `:setvar DemoPassword "1234"` as the first line, **Execute**.
   It rebuilds `OrderDesk` (about a minute). Rerun it whenever `sql\setup.sql`
   changes.
3. **Laptop, PowerShell:** enter `1234` when asked; expect `2000000`.

   ```powershell
   sqlcmd -S DESKTOP-NGU3456.mshome.net -U orderdesk -C -Q "SELECT COUNT(*) FROM report_rows"
   ```

## Part 2: DSN (the legacy workflow)

4. **Laptop, ODBC Data Sources (64-bit):** **User DSN > Add > ODBC Driver 18
   for SQL Server**. Name `OrderDeskDSN`, server `DESKTOP-NGU3456.mshome.net`,
   SQL Server authentication as `orderdesk`, default database `OrderDesk`,
   **Trust server certificate** checked. **Test Data Source** must succeed.

## Part 3: Configure and build

5. **Laptop, PowerShell, in this folder:** `copy orderdesk.cfg.example
   orderdesk.cfg`; in Notepad replace both `CHANGE_ME` with `1234` and
   `10.10.10.2` with `DESKTOP-NGU3456.mshome.net`.
6. **Laptop, Synergy x64 command prompt, in this folder:** `build`. It compiles
   `src\` into `bin\`: `orderdesk` (linked with `WND:tklib.elb`), `bench` and
   `errors`.
7. Commit the repo now. The show edits `src\orderdesk.dbl`, and `git restore
   src\orderdesk.dbl` puts the old settings back afterwards. Also keep a commit or
   branch with the three `%ssc_cmd` lines removed, as the fallback if the live
   Claude session stalls.

## Part 4: Try it

Start the UI app with `run` (UI Toolkit does not run under `dbs`) and the
console programs with `dbs bin\bench` and `dbs bin\errors`, from the repo root.

8. **Legacy line active:** `run`. Try **Reports > Nightly report**,
   **Orders > Post batch**, **Customers > Browse**, **Orders > New order**.
9. **Switch:** in `orderdesk.cfg`, put a `;` in front of the `VTX12_SQLNATIVE`
   line and remove it from the other `SQL_CONNECT` line. Save.
10. `run` again. It stops with "rejected 3 connection settings"; the
    messages are also in `orderdesk.log`.
11. Remove the three `%ssc_cmd` lines under `; Connection settings` in
    `src\orderdesk.dbl` (or let Claude do it), `build`, `run`. Run the
    report and the batch, then **Reports > Compare runs**.
12. `git restore src\orderdesk.dbl` and `build` to put the settings back.
13. Reset: `powershell -ExecutionPolicy Bypass -File scripts\reset-demo.ps1`. It warms
    up both drivers with `bench`, deletes the demo orders and runs, and makes
    the legacy line active.

## Part 5: Rehearse

14. Run the show three times; write down both drivers' numbers each time.
    Present your Windows ratios, not the Linux ones.
15. Rehearse the Claude segment with the exact prompt below, and time it.
16. Check the app is readable from the back row with two windows side by side.
    Start OrderDesk with `run` (not `dbr bin\orderdesk`): it sets
    `SYN_RESIZE_SCALE=1`, so maximizing the window scales its contents to the
    screen. The starting size is 175%; `set ORDERDESK_SCALE=` another percentage
    to change it. Menus, toolbar and message boxes follow Windows display
    scaling instead.
17. Turn Wi-Fi off and run everything except the Claude segment. Claude needs
    the internet: use a phone hotspot for that segment only.
18. Record one clean run as a fallback video.

## Part 6: Show day

Before going on: AC power, best-performance mode, notifications and updates
off; `scripts\reset-demo.ps1`; `git restore src\orderdesk.dbl` and `build`; two Synergy
command prompts side by side in this folder; `orderdesk.cfg` open in Notepad;
ODBC Data Sources on the User DSN tab; a Claude session open in this folder.

The show, about 7 minutes:

1. **Pain (1 min):** slide, then `OrderDeskDSN` > **Configure**, click through,
   **Cancel**.
2. **Before (1.5 min), left window:** `run`. Nightly report, then
   Post batch. Call out time and round trips.
3. **Switch (30 s):** move the `;` in `orderdesk.cfg`, save. Right window:
   `run`. Three settings rejected, each naming what to remove.
4. **Fix (2 min):** in the Claude session:

   > OrderDesk fails on the new SQL Connection driver with the errors in
   > orderdesk.log. Remove those settings from src\orderdesk.dbl.

   Review the diff, then `build` and `run` in the right window.
5. **After (1.5 min), right window:** Nightly report, Post batch, then
   **Reports > Compare runs** with both drivers side by side.
6. **Close (30 s):** one line plus three retired settings, no DSN, faster
   reads with far fewer round trips.

## If something goes wrong

| Symptom | Fix |
| --- | --- |
| "could not start: reading orderdesk.cfg" | Run from this folder; `orderdesk.cfg` needs one active `SQL_CONNECT=` line |
| "could not start: connecting" on `VTX12_SQLNATIVE` | **Test Data Source** on `OrderDeskDSN` |
| "could not start: connecting" on the new line | `Test-NetConnection DESKTOP-NGU3456.mshome.net -Port 1433`; "could not be loaded" means `rssql.dll` is missing |
| "rejected 3 connection settings" on legacy | Expected only on the new driver; check which line is active |
| "Counting round trips ... failed" | Rerun `sql\setup.sql` (it grants `VIEW SERVER STATE`) |
| Compare runs is empty | Run the report or batch first; `scripts\reset-demo.ps1` clears runs |
| Claude session stalls | Check out the fallback commit from step 7, `build`, carry on |
| Anything else | `scripts\reset-demo.ps1`; if still broken, play the recording |

## Files

```
orderdesk-demo\
  README.md
  build.bat               compiles src\ into bin\
  run.bat                 starts OrderDesk with scale-on-maximize
  orderdesk.cfg.example   both connect strings; copy to orderdesk.cfg
  src\
    orderdesk.dbl         UI Toolkit app
    bench.dbl             console timed steps
    errors.dbl            three failing statements and what the driver reports
    config.inc            reads the active line of orderdesk.cfg (all three)
    odstate.inc           OrderDesk's shared state
    theme.inc             OrderDesk's colour palette entries
  sql\
    setup.sql             database, login, grant, data and demo_runs table
    reset.sql             deletes demo orders and recorded runs
  scripts\
    reset-demo.ps1        warm-up on each driver, reset.sql, legacy line active
  assets\                 toolbar icons, from the UI Toolkit examples
  bin\                    build output (gitignored)
```

Run everything from the repo root: OrderDesk reads `orderdesk.cfg`, writes
`orderdesk.log` and loads `assets\` from the current folder (`run.bat` and
`build.bat` switch to it themselves). The `.dbl` files include `ODSRC:*.inc`;
`build.bat` sets `ODSRC` to `src\`.
