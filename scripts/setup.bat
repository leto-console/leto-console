@echo off
echo [leto-console] Собираем проект...
setlocal 
cd /d "%~dp0"
call preset_setup.bat leto-console win-st7735-debug
call preset_setup.bat leto-console stm32f411xe-st7735-debug
endlocal
