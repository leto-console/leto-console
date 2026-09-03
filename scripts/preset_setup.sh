#!/bin/bash

if [ -z "${LETO_PATH}" ]; then
    echo "[ERROR] Переменная окружения LETO_PATH не задана."
    echo "Запустите установочный скрипт или задайте её командой: export LETO_PATH=\"/путь/к/папке\""
    exit 1
fi

echo "[$1] Собираем конфигурацию $2"

(
    cd "$(dirname "${BASH_SOURCE[0]}")/.." || exit 1

    cmake --preset "$2"
    cmake --build --preset "$2" -j
    
    cmake --install "build/$2" --prefix "${LETO_PATH}/Common/$2" -component app
)
