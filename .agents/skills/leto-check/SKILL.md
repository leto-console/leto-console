---
name: leto-check
description: Прогнать проверки LETO перед коммитом — сборка затронутых репозиториев, ctest и проверка выгрузки DSO. Использовать как Definition of Done, для воспроизведения падения CI, при вопросе «что запускать перед коммитом».
---

# Проверки

Одна точка входа: `leto-console/scripts/leto-check.sh`.

```sh
./scripts/leto-check.sh          # затронутые репозитории: configure+build+ctest
./scripts/leto-check.sh --full   # все четыре + nm-проверка выгрузки .so
```

## Что внутри

1. `leto-sync`-инвариант: сборка идёт по цепочке `LetoAPI → LetoCore → LetoOS → leto-console`.
2. `ctest` есть только у `LetoCore` и `LetoOS` (пресет `linux-debug`):
   `ctest --test-dir "$LETO_PATH/$repo/build/linux-debug" --output-on-failure`.
3. Проверка выгрузки игры (только `--full`, нужен собранный `Apps/linux-debug/<Game>/`):

```sh
nm -C "$LETO_PATH/Apps/linux-debug/Battleship/libBattleship.so" | grep ' u ' | wc -l   # 0
```

Ненулевой вывод означает, что `-fno-gnu-unique` не попал в финальный `.so`
(наследование опции цели и пресета игры — разные вещи).

## Правила

- Не запускать полный прогон в цикле. Итерация — сборка одного репозитория;
  `leto-check` — перед коммитом; `--full` — перед PR и при правках загрузчика/CMake.
- CI собирает STM32, Windows и Linux. Linux (`build-linux` в `LetoOS`) — единственное
  место, где компилируется ветка загрузчика на `dlopen`; STM32 и Windows её не видят.
  Правка загрузчика без локального Linux-прогона непроверена.
- Задача готова, когда проверка зелёная. Дальше не перепроверять.
