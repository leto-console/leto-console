#!/bin/sh

# 1. Обработка флага -ssh
USE_SSH=0
for arg in "$@"; do
    if [ "$arg" = "-ssh" ]; then
        USE_SSH=1
        break
    fi
done

# Выбор базового URL в зависимости от флага
if [ $USE_SSH -eq 1 ]; then
    echo "Используются SSH URL для загрузки репозиториев..."
    BASE_URL="git@github.com:leto-console"
    SUFFIX=".git"
else
    echo "Используются HTTPS URL для загрузки репозиториев..."
    BASE_URL="https://github.com/leto-console"
    SUFFIX=""
fi

# 2. Клонирование репозиториев проекта LETO
git clone "${BASE_URL}/LetoAPI${SUFFIX}"
git clone "${BASE_URL}/LetoCore${SUFFIX}"
git clone "${BASE_URL}/leto-console${SUFFIX}"

mkdir -p Common
mkdir -p Apps

CURRENT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [ -n "$ZSH_VERSION" ] || echo "$SHELL" | grep -q "zsh"; then
    RC_FILE="$HOME/.zshrc"
else
    RC_FILE="$HOME/.bashrc"
fi

touch "$RC_FILE"

VAR_NAME="LETO_PATH"
NEW_LETO_LINE="export ${VAR_NAME}=\"${CURRENT_DIR}\""

if grep -q "export ${VAR_NAME}=" "$RC_FILE"; then
    grep -v "export ${VAR_NAME}=" "$RC_FILE" > "$RC_FILE.tmp"
    echo "${NEW_LETO_LINE}" >> "$RC_FILE.tmp"
    cat "$RC_FILE.tmp" > "$RC_FILE" && rm "$RC_FILE.tmp"
    echo "Переменная ${VAR_NAME} ОБНОВЛЕНА в $(basename "$RC_FILE") на: ${CURRENT_DIR}"
else
    echo "" >> "$RC_FILE"
    echo "${NEW_LETO_LINE}" >> "$RC_FILE"
    echo "Переменная ${VAR_NAME} ДОБАВЛЕНА в $(basename "$RC_FILE")."
fi

# 5. Интеграция внутренних путей LETO в системный PATH
PATH_LINE='export PATH="$LETO_PATH:$LETO_PATH/leto-console/scripts:$LETO_PATH/LetoCore/scripts:$PATH"'

if ! grep -q 'export PATH.*LETO_PATH' "$RC_FILE"; then
    echo "${PATH_LINE}" >> "$RC_FILE"
    echo "Пути LETO успешно интегрированы в системный PATH."
else
    grep -v 'export PATH.*LETO_PATH' "$RC_FILE" > "$RC_FILE.tmp"
    echo "${PATH_LINE}" >> "$RC_FILE.tmp"
    cat "$RC_FILE.tmp" > "$RC_FILE" && rm "$RC_FILE.tmp"
    echo "Пути LETO обновлены внутри $(basename "$RC_FILE")."
fi

echo "---"
echo "Скрипт завершил работу."
echo "Чтобы применить настройки прямо сейчас, выполните команду:"
echo "source $RC_FILE"
