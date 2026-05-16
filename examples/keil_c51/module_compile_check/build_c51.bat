@echo off
setlocal

set ROOT=%~dp0..\..\..
set BUILD=%~dp0build
set INCS=INCDIR(%ROOT%\core;%ROOT%\hal;%ROOT%\drivers;%ROOT%\utils;%ROOT%\protocols;%ROOT%\board\stc8h1k08_tssop20_demo)
set C51FLAGS=DEBUG
set ERR=0

if not exist "%BUILD%" mkdir "%BUILD%"

for %%F in ("%~dp0src\*.c") do call :compile "%%~fF" "%%~nF"

if "%ERR%"=="0" (
    echo.
    echo Keil C51 module compile check PASSED.
) else (
    echo.
    echo Keil C51 module compile check FAILED.
)

exit /b %ERR%

:compile
echo [C51] %~nx1
C51 "%~1" %INCS% %C51FLAGS% OBJECT("%BUILD%\%~2.obj") PRINT("%BUILD%\%~2.lst")
if errorlevel 1 set ERR=1
exit /b 0
