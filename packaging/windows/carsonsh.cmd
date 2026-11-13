@echo off
setlocal
if defined USERPROFILE set "HOME=%USERPROFILE%"
"%~dp0carsonsh.exe" %*
exit /b %ERRORLEVEL%
