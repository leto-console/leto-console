---
name: leto-new-scene
description: Добавить новый экран/сцену в прошивку LETO — SceneID, регистрация билдера, навигация. Использовать при задачах вида «добавить сцену», «новый экран», «пункт меню», «UI_Object в консоли».
---

# Новая сцена в leto-console

Сцены живут в `Core/Inc/ProjectScenes` и `Core/Src/ProjectScenes`. Интерфейс — `IScene`
из `LetoCore` (`SceneManager/IScene.hpp`). Системный менеджер — `SystemSceneManager`
из `LetoOS/include/System/`.

## Шаги

1. Добавить идентификатор в `enum class SceneID` (`Core/Inc/ProjectScenes/SceneID.hpp`)
   внутрь диапазона `__SCREENS_START` … `__SCREENS_COUNT` — по границам enum строится
   таблица сцен.
2. Создать `FooScene.hpp`/`FooScene.cpp` рядом с существующими (`MainScene`, `EyesScene`,
   `DebugScene`): наследование от `IScene`, `Draw`, обработка `ButtonEvent`.
3. Зарегистрировать билдер в `Core/Src/Application/System.cpp`:
   `SystemSceneManager::Instance().AddSceneBuilder<FooScene>(SceneID::FOO);`
   Пользовательские сцены — в `InitUserScenes()`, системные — в системной ветке ниже.
   `InitUserScenes()` вызывается только после успешного входа: сцена, нужная до
   авторизации, регистрируется в системной части.
4. Навигацию добавлять от существующей сцены (`ChangeScene`), а не отдельным путём.
5. Статическое состояние сцены — статический член с фиксированной ёмкостью
   (`StaticList`/`StaticTextView`), не динамическая память: на MCU её нет.

## Инварианты

- Сцена консоли — это `leto-console`, не `LetoCore` и не `LetoOS`. Общая логика
  для игр идёт в `LetoCore`, системная — в `LetoOS`.
- В играх доступно только `LetoAPI`; заголовки `LetoOS` из `LetoCore` не подключаются.

## Проверка

```sh
cd "$LETO_PATH/leto-console" && cmake --build build/linux-st7735-debug -j
```

Затем `leto-run-game` (headless). Сцена считается добавленной, когда она реально
открывается, а не когда компилируется. Прошивочные пресеты — `leto-build`.
