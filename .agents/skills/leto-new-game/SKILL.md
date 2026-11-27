---
name: leto-new-game
description: Создать или изменить игру для LETO на базе leto-games/game-template, собрать её под Linux/ПК и положить туда, где загрузчик её найдёт. Использовать при задачах в организации leto-games, при правках game-template, Battleship и при добавлении фикстур для тестов загрузчика.
---

# Игра для LETO

Игры живут в организации `leto-games` (`game-template`, `Battleship`), а не в
`leto-console`.

## Начало

```sh
git clone https://github.com/leto-games/game-template.git MyGame
cd MyGame && cmake --list-presets          # linux-пресет шаблона
cmake --preset <linux-пресет> && cmake --build "build/<linux-пресет>" -j
```

Точка входа — экспорт `__leto_exec_info__` (не функция `Init`), см. `game-template/source/Export.hpp`
и `game-template/source/TemplateGame.cpp`. `stubs.cpp` — заглушки хостовых символов для линковки.

## Обязательные условия

- Игра линкует только `LetoAPI` + `LetoCore`; `LetoOS` в игру не попадает.
- `-fno-gnu-unique` должен попасть в финальный `.so` (в пресетах `linux-base`):
  иначе `dlclose` не выгружает игру, glibc держит DSO замапленным из-за `STB_GNU_UNIQUE`.
- Доступ к состоянию хоста — только через `leto_api_v1` (например,
  `leto_api_v1->Font->FindFont`), а не через собственные глобалы: копия `LetoCore`
  внутри `.so` имеет свои границы.

## Куда кладётся сборка

Загрузчик ищет в каскаде `<cwd>/Apps/<preset>/`, `<cwd>/Apps/`,
`$LETO_PATH/Apps/<preset>/`, `$LETO_PATH/Apps/`. Пресет — из `-DLETO_PRESET_NAME`.
`cmake --install build/linux-debug --prefix "$LETO_PATH/Apps/linux-debug/MyGame" --component app`.

## Проверка

```sh
nm -C "$LETO_PATH/Apps/linux-debug/MyGame/libMyGame.so" | grep ' u ' | wc -l   # 0
```

Затем `leto-run-game` с `--game <путь к .so>`. Пустой экран без падения — не успех:
нужен наблюдаемый эффект.

## Права

В `leto-games` пишут `GITHUB_TOKEN` и `LETO_GAMES_TOKEN` (fine-grained). В
`leto-console/*` — только `GH_PROJECTS_TOKEN`. Процесс PR — `leto-pr`.
