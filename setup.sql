-- Builds the OrderDesk database from scratch. Run as a sysadmin; re-running drops and rebuilds it.
--   sqlcmd -S 10.10.10.2 -U sa -C -i setup.sql -v DemoPassword="<demo password>"

USE master;
GO
IF DB_ID('OrderDesk') IS NOT NULL
BEGIN
    ALTER DATABASE OrderDesk SET SINGLE_USER WITH ROLLBACK IMMEDIATE;
    DROP DATABASE OrderDesk;
END;
GO
CREATE DATABASE OrderDesk;
GO
IF SUSER_ID('orderdesk') IS NULL
    CREATE LOGIN orderdesk WITH PASSWORD = '$(DemoPassword)', DEFAULT_DATABASE = OrderDesk;
ELSE
    ALTER LOGIN orderdesk WITH PASSWORD = '$(DemoPassword)', DEFAULT_DATABASE = OrderDesk;
-- Lets OrderDesk read its own network counters from sys.dm_exec_connections.
GRANT VIEW SERVER STATE TO orderdesk;
GO

USE OrderDesk;
GO
CREATE USER orderdesk FOR LOGIN orderdesk;
ALTER ROLE db_datareader ADD MEMBER orderdesk;
ALTER ROLE db_datawriter ADD MEMBER orderdesk;
GO

CREATE TABLE dbo.customers (
    id      int             NOT NULL PRIMARY KEY,
    name    varchar(40)     NOT NULL
);
INSERT INTO dbo.customers (id, name) VALUES
    (1001, 'Acme Corp'),            (1002, 'Globex'),
    (1003, 'Initech'),              (1004, 'Contoso'),
    (1005, 'Northwind Traders'),    (1006, 'Fabrikam'),
    (1007, 'Tailspin Toys'),        (1008, 'Wide World Importers'),
    (1009, 'Adventure Works'),      (1010, 'Litware'),
    (1011, 'Proseware'),            (1012, 'Lucerne Publishing'),
    (1013, 'Margie''s Travel'),     (1014, 'Woodgrove Bank'),
    (1015, 'Blue Yonder Airlines'), (1016, 'Coho Winery'),
    (1017, 'Fourth Coffee'),        (1018, 'Alpine Ski House'),
    (1019, 'Trey Research'),        (1020, 'Wingtip Toys');

CREATE TABLE dbo.orders (
    order_no    int             NOT NULL CONSTRAINT PK_orders PRIMARY KEY,
    customer_id int             NOT NULL REFERENCES dbo.customers (id),
    placed_at   datetime2(0)    NOT NULL DEFAULT SYSDATETIME()
);
INSERT INTO dbo.orders (order_no, customer_id) VALUES (10420, 1001);

-- One row per timed step run from the OrderDesk app, for Reports > Compare runs.
CREATE TABLE dbo.demo_runs (
    id          int             IDENTITY NOT NULL PRIMARY KEY,
    run_at      datetime2(0)    NOT NULL DEFAULT SYSDATETIME(),
    driver      varchar(40)     NOT NULL,
    step        varchar(10)     NOT NULL,
    row_count   int             NOT NULL,
    tenths      int             NOT NULL,
    round_trips bigint          NOT NULL
);

CREATE TABLE dbo.report_rows (
    id      int             NOT NULL PRIMARY KEY,
    amount  decimal(18,4)   NOT NULL,
    payload varchar(128)    NOT NULL
);
WITH digits(n) AS (
    SELECT n FROM (VALUES (0),(1),(2),(3),(4),(5),(6),(7),(8),(9)) d(n)
), numbers AS (
    SELECT 1 + a.n + 10*b.n + 100*c.n + 1000*d.n + 10000*e.n + 100000*f.n AS id
    FROM digits a CROSS JOIN digits b CROSS JOIN digits c
    CROSS JOIN digits d CROSS JOIN digits e CROSS JOIN digits f
)
INSERT INTO dbo.report_rows (id, amount, payload)
SELECT id, CONVERT(decimal(18,4), id * CONVERT(decimal(18,4), 0.0001)), REPLICATE('X', 128)
FROM numbers
WHERE id <= 1000000;
GO
