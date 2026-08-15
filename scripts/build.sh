SCRIPTS=$(pwd)
cd ../../
"$SCRIPTS"/item_build.sh LetoAPI stm32f411xe-debug
"$SCRIPTS"/item_build.sh LetoCore stm32f411xe-debug
"$SCRIPTS"/item_build.sh leto-console stm32f411xe-st7735-debug

