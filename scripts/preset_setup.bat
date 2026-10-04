@echo off
if "%LETO_PATH%"=="" (
	echo [ERROR] Переменная окружения LETO_PATH не задана.
	echo Задайте её один раз командой: setx LETO_PATH "C:/путь/к/папке/с/репозиториями"
	exit /b 1
)
echo [%~1] Собираем конфигурацию %~2
setlocal enabledelayedexpansion
cd /d "%~dp0/.." || exit /b !errorlevel!
cmake --preset %~2 || exit /b !errorlevel!
cmake --build --preset %~2 -j || exit /b !errorlevel!
cmake --install build/%~2 --prefix "%LETO_PATH%Console/%~2" --component app || exit /b !errorlevel!
endlocal
