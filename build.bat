@echo off
rem Builds the demo programs. Run from a Synergy x64 command prompt in this folder.
dbl orderdesk && dblink orderdesk WND:tklib.elb || exit /b 1
dbl bench && dblink bench || exit /b 1
dbl errors && dblink errors || exit /b 1
echo Built orderdesk, bench and errors.
