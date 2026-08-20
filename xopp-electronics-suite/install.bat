@echo off
echo ========================================================
echo   Xournal++ Electronics Engineering Suite Installer
echo ========================================================

:: Check for Python
python --version >nul 2>&1
IF %ERRORLEVEL% EQU 0 (
    echo [INFO] Python found. Generating assets procedurally...
    python build_assets.py
) ELSE (
    echo [INFO] Python not found. Assuming assets are already built.
)

set "XOPP_DIR=%LOCALAPPDATA%\xournalpp"
echo [INFO] Target Directory: %XOPP_DIR%

IF NOT EXIST "%XOPP_DIR%" (
    echo [ERROR] Xournal++ configuration directory not found. Please run Xournal++ at least once before installing.
    pause
    goto :EOF
)

:: Backup existing configs safely
echo [INFO] Backing up existing toolbar.ini and palette.gpl...
IF EXIST "%XOPP_DIR%\toolbar.ini" (
    copy /y "%XOPP_DIR%\toolbar.ini" "%XOPP_DIR%\toolbar.ini.bak" >nul
)
IF EXIST "%XOPP_DIR%\palette.gpl" (
    copy /y "%XOPP_DIR%\palette.gpl" "%XOPP_DIR%\palette.gpl.bak" >nul
)

:: Deploy New Configs
echo [INFO] Deploying Engineering Toolbar and Palette...
copy /y config\toolbar.ini "%XOPP_DIR%\toolbar.ini" >nul
copy /y config\palette.gpl "%XOPP_DIR%\palette.gpl" >nul

:: Deploy Plugin
echo [INFO] Deploying Electronics Suite Plugin...
xcopy /s /e /y /i plugins\electronics-suite "%XOPP_DIR%\plugins\electronics-suite" >nul

:: Instructions for Settings
echo.
echo ========================================================
echo   INSTALLATION COMPLETE!
echo ========================================================
echo.
echo Next Steps for Optimal Palm Rejection on 2-in-1 Windows Devices:
echo 1. Open Xournal++
echo 2. Go to Edit -^> Preferences -^> Input System
echo 3. Set your stylus to "Pen"
echo 4. Set "Touchscreen" to "Touch"
echo 5. Go to the "Touchscreen" settings tab and check:
echo    "Disable drawing with touchscreen (only panning/zooming)"
echo 6. Enjoy focused, distraction-free note-taking!
echo.
pause
