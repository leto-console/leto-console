@echo off
if "%LETO_PATH%"=="" (
	echo [ERROR] Переменная окружения LETO_PATH не задана.
	echo Задайте её один раз командой: setx LETO_PATH "C:/путь/к/папке/с/репозиториями"
	exit /b 1
)
cd /d "%~1"
cmake --preset %~2
cmake --build -j --preset %~2
