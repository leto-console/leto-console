#!/bin/bash

echo "[LetoAPI] Собираем проект..."
set -e

(
    cd "$(dirname "${BASH_SOURCE}")" || exit 1

    ./preset_setup.sh LetoAPI linux-st7735-debug
    ./preset_setup.sh LetoAPI stm32f411xe-st7735-debug
)
