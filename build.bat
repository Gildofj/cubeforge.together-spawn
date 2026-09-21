@echo off
setlocal enabledelayedexpansion

echo =======================================================
echo  CubeForge TogetherSpawn - Automated MSVC x64 Build
echo =======================================================

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0build.ps1" %*
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed with error code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo [SUCCESS] Build process completed.
exit /b 0
