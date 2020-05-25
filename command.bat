@echo off
cd /D "H:\cpp-projects\ide"
CALL ..\libs.bat
echo %CD%

cmd /c g++ ide.cpp %ADV% -I%CONSOLE% %WCONSOLE% -Iwindows -o ide.exe -w -g
if "%errorlevel%" NEQ "0" (
pause
exit /b
)
ide.exe
SET RRR=%errorlevel%
if "%errorlevel%" NEQ "0" (
echo Exited with error code %errorlevel%
echo Stack overflow -1073741571 C00000FD
echo Access violation or divide by 0 -1073741676 C0000094
echo Map error SIGSEGV -1073741811 C000000D
pause
exit /b
)
