if [ -z "$LETO_PATH" ]; then
	echo "[ERROR] Переменная окружения LETO_PATH не задана."
	echo "Задайте её, например, в ~/.bashrc: export LETO_PATH=\"/путь/к/папке/с/репозиториями\""
	exit 1
fi
cd $1
cmake --preset $2
cmake --build -j --preset $2
