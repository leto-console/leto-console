#!/bin/bash

git clone https://github.com/leto-console/LetoAPI.git
git clone https://github.com/leto-console/LetoCore.git
git clone https://github.com/leto-console/leto-console.git

mkdir -p Common
mkdir -p Apps

CURRENT_DIR="$(cd "$(dirname "${BASH_SOURCE}")" && pwd)"

BASHRC_FILE="$HOME/.bashrc"

VAR_NAME="LETO_PATH"
NEW_LETO_LINE="export ${VAR_NAME}=\"${CURRENT_DIR}\""

if grep -q "export ${VAR_NAME}=" "$BASHRC_FILE"; then
    sed -i "s|export ${VAR_NAME}=.*|${NEW_LETO_LINE}|" "$BASHRC_FILE"
    echo "Переменная ${VAR_NAME} ОБНОВЛЕНА на: ${CURRENT_DIR}"
else
    echo "" >> "$BASHRC_FILE"
    echo "${NEW_LETO_LINE}" >> "$BASHRC_FILE"
    echo "Переменная ${VAR_NAME} ДОБАВЛЕНА."
fi

# Добавляем корневую папку LETO и папки со скриптами из репозиториев
PATH_LINE='export PATH="$LETO_PATH:$LETO_PATH/leto-console/scripts:$LETO_PATH/LetoCore/scripts:$PATH"'

if ! grep -q 'export PATH.*LETO_PATH' "$BASHRC_FILE"; then
    echo "${PATH_LINE}" >> "$BASHRC_FILE"
    echo "Пути LETO успешно интегрированы в системный PATH."
else
    echo "Пути LETO уже присутствуют в вашем PATH внутри .bashrc."
fi

echo "---"
echo "Скрипт завершил работу."
echo "Чтобы применить настройки прямо сейчас, выполните команду: source ~/.bashrc"
