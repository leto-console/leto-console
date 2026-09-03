#!/bin/bash

echo "[LetoAPI] Собираем проект..."

(
    cd "$(dirname "${BASH_SOURCE}")" || exit 1

    #./preset_setup.sh LetoAPI ubuntu-debug
    ./preset_setup.sh LetoAPI stm32f411xe-st7735-debug
)
