@echo off
set "SCRIPTS=%cd%"
cd /d "..\..\"
call "%SCRIPTS%\item_build.bat" LetoAPI stm32f411xe-debug
call "%SCRIPTS%\item_build.bat" LetoCore stm32f411xe-debug
call "%SCRIPTS%\item_build.bat" leto-console stm32f411xe-st7735-debug

call "%SCRIPTS%\item_build.bat" LetoAPI win-debug
call "%SCRIPTS%\item_build.bat" LetoCore win-debug
call "%SCRIPTS%\item_build.bat" leto-console win-debug-st7735

