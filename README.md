# OrderDesk: SQL Connection demo

Demo for the DevPartner 2026 session "Rebuilding SQL Connection from the Ground Up".

- `src\orderdesk.dbl` is a UI Toolkit order-entry app: browse customers, place an
  order, post a batch of 50,000 orders, run a 2,000,000-row nightly report, and
  compare the timed runs of both drivers.
- `src\bench.dbl` is the console version of the timed steps.
- `src\errors.dbl` runs three failing statements and prints what the driver
  reports.

Switching between the legacy SQL Connection driver and the new one is one line
in `orderdesk.cfg`.

What the demo shows:

- **Drop-in.** The same `.dbr` runs on both drivers after a one-line config
  change.
- **No DSN, no ODBC.** The new connect string names the server directly.
- **Reads.** Each timed run reports its time and the network round trips
  SQL Server counted for the session.
- **Writes.** 50,000 orders through one prepared insert, committed every 1,000.
- **Errors** (`errors.dbl`). Same SQLCODE for ordinary SQL errors, with the
  server's message; for a procedure call SQL Server rejects, the legacy driver
  reports `-1` "not found" while the new driver reports the real error.

## Requirements

- Synergy/DE x64 with SQL Connection and UI Toolkit, including the new SQL
  Server driver. Connect strings starting `sqlserver:` use the new driver; the
  legacy `VTX12_SQLNATIVE:` strings use the Trifox driver.
- Microsoft ODBC Driver 18 for SQL Server (x64), for the legacy driver.
- SQL Server 2019 or later with SQL Server authentication enabled and TCP 1433
  reachable from the client.

In the steps below, `<server>` is the SQL Server host and `<password>` is a
password you choose for the demo login.

## 1. Database

1. In SSMS, connected as a sysadmin: **New Query**, paste `sql\setup.sql`, turn
   on **Query > SQLCMD Mode**, add `:setvar DemoPassword "<password>"` as the
   first line, and **Execute**. It drops and rebuilds the `OrderDesk` database
   and creates the `orderdesk` login.
2. Check it from the client; expect `2000000`:

   ```powershell
   sqlcmd -S <server> -U orderdesk -C -Q "SELECT COUNT(*) FROM report_rows"
   ```

## 2. DSN for the legacy driver

3. **ODBC Data Sources (64-bit)** > **User DSN > Add > ODBC Driver 18 for SQL
   Server**. Name `OrderDeskDSN`, server `<server>`, SQL Server authentication
   as `orderdesk`, default database `OrderDesk`, **Trust server certificate**
   checked. **Test Data Source** must succeed.

## 3. Configure and build

4. `copy orderdesk.cfg.example orderdesk.cfg`, then replace both `CHANGE_ME`
   with `<password>` and `your-sql-server` with `<server>`. `orderdesk.cfg` is
   gitignored.
5. In a Synergy/DE x64 command prompt, in this folder: `build`. It compiles
   `src\` into `bin\`: `orderdesk` (linked with `WND:tklib.elb`), `bench` and
   `errors`.

## 4. Run

Start the UI app with `run` (UI Toolkit does not run under `dbs`) and the
console programs with `dbs bin\bench` and `dbs bin\errors`, from the repo root.
`run` sets `SYN_RESIZE_SCALE=1`, so maximizing the window scales its contents to
the screen. The starting size is 175%; `set ORDERDESK_SCALE=` another
percentage to change it.

6. With the `VTX12_SQLNATIVE` line active: `run`. Try **Reports > Nightly
   report**, **Orders > Post batch**, **Customers > Browse** and **Orders > New
   order**.
7. In `orderdesk.cfg`, put a `;` in front of the `VTX12_SQLNATIVE` line and
   remove it from the other `SQL_CONNECT` line. Save.
8. `run` again. Run the report and the batch, then **Reports > Compare runs**.
9. **File > Start fresh** deletes the recorded runs and demo orders, so the
   scoreboard and Compare runs start empty.
   `powershell -ExecutionPolicy Bypass -File scripts\reset-demo.ps1` does the
   same, runs `bench` once on each driver so both start with SQL Server's cache
   warm, and makes the legacy line active.

## Troubleshooting

| Symptom | Fix |
| --- | --- |
| "could not start: reading orderdesk.cfg" | Run from this folder; `orderdesk.cfg` needs one active `SQL_CONNECT=` line |
| "could not start: connecting" on `VTX12_SQLNATIVE` | **Test Data Source** on `OrderDeskDSN` |
| "could not start: connecting" on the new line | `Test-NetConnection <server> -Port 1433`; "could not be loaded" means the new driver's library is missing |
| "Counting round trips ... failed" | Rerun `sql\setup.sql` (it grants `VIEW SERVER STATE`) |
| Compare runs is empty | Run the report or batch first |

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
    reset-demo.ps1        resets the data and runs bench on each driver
  assets\                 toolbar icons, from the UI Toolkit examples
  bin\                    build output (gitignored)
```

Run everything from the repo root: OrderDesk reads `orderdesk.cfg` and loads
`assets\` from the current folder (`run.bat` and `build.bat` switch to it
themselves). The `.dbl` files include `ODSRC:*.inc`; `build.bat` sets `ODSRC`
to `src\`.
