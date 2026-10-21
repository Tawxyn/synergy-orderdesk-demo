# OrderDesk: SQL Connection demo

Demo for the DevPartner 2026 session "Rebuilding SQL Connection from the Ground Up".

- `src\orderdesk.dbl` is a UI Toolkit order-entry app: browse customers, place an
  order, post a batch of 50,000 orders, run a 2,000,000-row nightly report, and
  compare the timed runs of both drivers.
- `src\bench.dbl` is the console version of the timed steps.
- `src\errors.dbl` runs three failing statements and prints what the driver
  reports.

Switching between the legacy SQL Connection driver and the new one is one line
in `orderdesk.cfg`. SQL Server runs in a Hyper-V VM on the same laptop, on the
Default Switch, reached by the name `DESKTOP-NGU3456.mshome.net`. Nothing is
copied into the VM.

What the demo shows:

- **Drop-in.** The same `.dbr` runs on both drivers after a one-line config
  change.
- **No DSN, no ODBC.** The new connect string names the server directly.
  Trifox still loads at `%SSC_INIT`.
- **Reads.** Fewer round trips and a shorter time. Measured on Linux: about 11x
  fewer round trips and 2.4x faster.
- **Writes.** 50,000 orders through one prepared insert, committed every
  1,000. Measured on Linux, reused inserts took about the same time on both
  drivers.
- **Errors** (`errors.dbl`). Same SQLCODE for ordinary SQL errors with
  the server's message; for a procedure call SQL Server rejects, legacy reports
  `-1` "not found" while the new driver reports the real error.

## Build requirements

A Synergy/DE x64 build with the driver-selection runtime (both drivers load
side by side; `sqlserver:` goes to `rssql.dll`), `project/ssql-rust` (server
error text), `rssql/perf` (read
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

## Part 4: Try it

Start the UI app with `run` (UI Toolkit does not run under `dbs`) and the
console programs with `dbs bin\bench` and `dbs bin\errors`, from the repo root.
`run` sets `SYN_RESIZE_SCALE=1`, so maximizing the window scales its contents to
the screen. The starting size is 175%; `set ORDERDESK_SCALE=` another
percentage to change it. Menus, toolbar and message boxes follow Windows
display scaling instead.

7. **Legacy line active:** `run`. Try **Reports > Nightly report**,
   **Orders > Post batch**, **Customers > Browse**, **Orders > New order**.
8. **Switch:** in `orderdesk.cfg`, put a `;` in front of the `VTX12_SQLNATIVE`
   line and remove it from the other `SQL_CONNECT` line. Save.
9. `run` again. Run the report and the batch, then **Reports > Compare runs**.
10. **File > Start fresh** deletes the recorded runs and demo orders, so the
    scoreboard and Compare runs start empty.
11. Reset: `powershell -ExecutionPolicy Bypass -File scripts\reset-demo.ps1`. It warms
    up both drivers with `bench`, deletes the demo orders and runs, and makes
    the legacy line active.

## If something goes wrong

| Symptom | Fix |
| --- | --- |
| "could not start: reading orderdesk.cfg" | Run from this folder; `orderdesk.cfg` needs one active `SQL_CONNECT=` line |
| "could not start: connecting" on `VTX12_SQLNATIVE` | **Test Data Source** on `OrderDeskDSN` |
| "could not start: connecting" on the new line | `Test-NetConnection DESKTOP-NGU3456.mshome.net -Port 1433`; "could not be loaded" means `rssql.dll` is missing |
| "Counting round trips ... failed" | Rerun `sql\setup.sql` (it grants `VIEW SERVER STATE`) |
| Compare runs is empty | Run the report or batch first; **File > Start fresh** and `scripts\reset-demo.ps1` clear runs |
| Anything else | `scripts\reset-demo.ps1` |

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

Run everything from the repo root: OrderDesk reads `orderdesk.cfg` and loads
`assets\` from the current folder (`run.bat` and
`build.bat` switch to it themselves). The `.dbl` files include `ODSRC:*.inc`;
`build.bat` sets `ODSRC` to `src\`.
