@echo off
setlocal
set "HOME=%~dp0home"
if not exist "%HOME%\.config\carsonsh" mkdir "%HOME%\.config\carsonsh"
"%~dp0carsonsh.exe" %*
exit /b %ERRORLEVEL%
