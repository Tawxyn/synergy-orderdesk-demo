@echo off
rem Starts OrderDesk from this folder. Maximizing the window scales its contents to the screen.
setlocal
cd /d "%~dp0"
set SYN_RESIZE_SCALE=1
dbr bin\orderdesk
