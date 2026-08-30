# Инструкция по разворачиванию Leto SDK

## Разработка на Windows

### 1. Установка инструментов сборки и версионирования кода

#### 1.1 Необходимые инструменты:
1. Build Tools для Visual Studio (для сборки ПО под Windows)
2. GNU Arm Embedded Toolchain (для сборки ПО под STM32)
3. Ninja
4. CMake
5. Git

#### 1.2 Ссылки:
1. https://visualstudio.microsoft.com/ru/downloads/
2. https://gitlab.arm.com/tooling/gnu-toolchains-for-arm
3. https://github.com/ninja-build/ninja
4. https://cmake.org/download/
5. https://git-scm.com/install/windows

#### 1.3 Примечания:
2. После установки GNU Arm Embedded Toolchain необходимо будет добавить путь к папке bin (с файлами arm-none-eabi-\*.exe) в переменную окружения PATH

#### 1.4 Проверка корректности установки инструментов

Выполните следующие команды в терминале:
```bash
cmake --version
ninja --version
arm-none-eabi-gcc --version
git --version
```
Все команды должны вернуть номера версий без ошибок.

### 2. Установка и настройка SDK
1. Создать папку для SDK
2. Скопировать скрипты `clone_env.bat`, `setup.bat` в корень папки для SDK
3. Найти и запустить `x64 Native Tools Command Prompt for VS`
4. Перейти в папку с SDK и выполнить скрипт `clone_env.bat`
5. Перезапустить терминал (закрыть и снова выполнить п.3)
6. Выполнить скрипт `setup.bat`

### 3. Настройка переменных окружения:
Необходимо в переменную окружения `%Path%` (можно как в системную, так и пользовательскую)
добавить путь к динамическим библиотекам Leto: 
```
%LETO_PATH%Common\win-debug\bin
```
Переменная окружения `LETO_PATH` при настройке SDK устанавливается автоматически. 

### 4. Проверка корректности установки SDK
Запустите файл `leto-console.exe` из папки `%LETO_PATH%\Console\win-st7735-debug\bin`.
Программа должна запустить без ошибок, отобразив окно игровой консоли.


## Разработка на Linux 

🚧 *Этот раздел находится в разработке. Информация будет добавлена позже.* 🚧
