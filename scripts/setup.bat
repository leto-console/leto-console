@echo off
echo [leto-console] Собираем проект...
setlocal enabledelayedexpansion
cd /d "%~dp0" || exit /b !errorlevel!
call preset_setup.bat leto-console win-st7735-debug || exit /b !errorlevel!
call preset_setup.bat leto-console stm32f411xe-st7735-debug || exit /b !errorlevel!
endlocal
