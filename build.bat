@echo off
rem Builds the demo programs into bin\. Run from a Synergy x64 command prompt.
setlocal
cd /d "%~dp0"
set ODSRC=%~dp0src\
if not exist bin mkdir bin
dbl -o bin\orderdesk src\orderdesk && dblink -o bin\orderdesk.dbr bin\orderdesk WND:tklib.elb || exit /b 1
dbl -o bin\bench src\bench && dblink -o bin\bench.dbr bin\bench || exit /b 1
dbl -o bin\errors src\errors && dblink -o bin\errors.dbr bin\errors || exit /b 1
echo Built bin\orderdesk.dbr, bin\bench.dbr and bin\errors.dbr.
