@echo off
chcp 65001 > nul

:: 1. Обработка флага -ssh
set USE_SSH=0
:loop
if "%~1"=="" goto end_loop
if "%~1"=="-ssh" (
    set USE_SSH=1
    goto end_loop
)
shift
goto loop
:end_loop

:: Выбор базового URL в зависимости от флага
if %USE_SSH%==1 (
    echo Используются SSH URL для загрузки репозиториев...
    set "BASE_URL=git@github.com:leto-console"
    set "SUFFIX=.git"
) else (
    echo Используются HTTPS URL для загрузки репозиториев...
    set "BASE_URL=https://github.com/leto-console"
    set "SUFFIX="
)

:: 2. Клонирование репозиториев проекта LETO
git clone "%BASE_URL%/LetoAPI%SUFFIX%"
git clone "%BASE_URL%/LetoCore%SUFFIX%"
git clone "%BASE_URL%/LetoOS%SUFFIX%"
git clone "%BASE_URL%/leto-console%SUFFIX%"

mkdir Common
mkdir Apps

setx LETO_PATH %~dp0
