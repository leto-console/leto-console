chcp 65001 > nul

call ./LetoAPI/scripts/setup.bat || exit /b 1
call ./LetoCore/scripts/setup.bat || exit /b 1
call ./LetoOS/scripts/setup.bat || exit /b 1
call ./leto-console/scripts/setup.bat || exit /b 1
echo.
echo [LetoSDK] Сборка успешно завершена!
