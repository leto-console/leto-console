В данной директории представлены скрипты для локальной сборки проекта.

Для локальной сборки используется **системная переменная окружения** `LETO_PATH`,
которая ведет к папке с остальными репозиториями (LetoAPI, LetoCore, leto-console, игры).

CMake читает её как `$ENV{LETO_PATH}`: если переменная задана — подключаются готовые
локальные артефакты (include/lib соседних репозиториев), если нет — выполняется
серверная сборка зависимостей через FetchContent.

## Установка переменной

Windows (один раз, затем перезапустить терминал/IDE):

```bat
setx LETO_PATH "C:/Users/<user>/.../STM32Projects"
```

Linux / macOS / Termux (добавить в `~/.bashrc` или `~/.zshrc`):

```sh
export LETO_PATH="/home/<user>/.../STM32Projects"
```

## Проверки и обслуживание базы знаний

Для агентов (и для людей) точка входа одна — `leto-check.sh`. Он собирает
репозитории по цепочке `LetoAPI → LetoCore → LetoOS → leto-console` и прогоняет
`ctest` там, где тесты есть: `LetoCore` (116) и `LetoOS` (16), только `linux-*`.

```sh
./scripts/leto-check.sh                # изменённые репозитории и всё, что ниже по цепочке
./scripts/leto-check.sh --full         # все четыре + проверка выгрузки игры из .so
./scripts/leto-check.sh --repo LetoOS  # явно указанный репозиторий
```

`--full` проверяет, что в собранной игре нет символов `STB_GNU_UNIQUE`: с ними
glibc держит библиотеку замапленной и `dlclose` её не выгружает (нужен
`-fno-gnu-unique` в пресете игры).

| Скрипт | Назначение |
| --- | --- |
| `leto-check.sh` | сборка и тесты одной командой, с расширением по цепочке зависимостей |
| `ctx_lint.py` | линтер базы знаний: пути, пресеты, frontmatter скиллов, бюджеты размера |
| `gen_reference.py` | генерирует `docs/reference/presets.md` из `CMakePresets.json` (`--check` — без записи) |
| `sync-skills.sh` | копирует `.agents/skills` в `LetoAPI`, `LetoCore`, `LetoOS` (`--check` — сравнение) |

`ctx_lint.py` и `gen_reference.py --check` запускаются в CI джобой `ctx-lint`:
переименованный пресет или удалённый путь не доедет до `main`.

Правила расширения базы знаний описаны в `AGENTS.md`, решения — в `docs/adr/`.
