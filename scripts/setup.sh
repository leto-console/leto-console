#!/bin/bash

echo "[leto-console] Собираем проект..."
set -e

(
    cd "$(dirname "${BASH_SOURCE}")" || exit 1

    ./preset_setup.sh leto-console linux-st7735-debug
    ./preset_setup.sh leto-console stm32f411xe-st7735-debug
)
