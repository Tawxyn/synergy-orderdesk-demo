@echo off
rem Starts OrderDesk from this folder. Maximizing the window scales its contents to the screen.
setlocal
cd /d "%~dp0"
if not defined SQL_CONNECT (
    echo Set SQL_CONNECT in this command prompt before starting OrderDesk.
    exit /b 1
)
set SYN_RESIZE_SCALE=1
dbr bin\orderdesk
