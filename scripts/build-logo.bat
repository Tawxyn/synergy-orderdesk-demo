@echo off
setlocal
rem Use the current x64 C++ tools, or find an installed Visual Studio toolchain.
where cl >nul 2>&1
if not errorlevel 1 if not "%VSCMD_ARG_TGT_ARCH%"=="x86" goto compile
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto missing
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "LOGO_VS=%%i"
if not defined LOGO_VS goto missing
call "%LOGO_VS%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
:compile
cl /nologo /LD /MT /EHsc /std:c++17 /W4 /WX /O2 /Fo:bin\logo.obj src\logo.cpp /link /MACHINE:X64 /OUT:bin\orderlogo.dll /IMPLIB:bin\orderlogo.lib user32.lib gdi32.lib comctl32.lib
exit /b %errorlevel%
:missing
echo The pixel logo needs the Visual Studio C++ x64 build tools and Windows SDK.
exit /b 1
