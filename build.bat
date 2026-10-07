@echo off
setlocal enabledelayedexpansion

echo ======================================================================
echo          RinganBrowser - Build Script (Native C++20 Win32)
echo ======================================================================
echo.

set COMPILER_DIR=C:\w64devkit\bin

if not exist "%COMPILER_DIR%\g++.exe" (
    echo [ERROR] G++ compiler tidak ditemukan di %COMPILER_DIR%!
    pause
    exit /b 1
)

echo [1/3] Mengompilasi Resource dan Manifest (windres)...
"%COMPILER_DIR%\windres.exe" resource.rc -O coff -o resource.res
if errorlevel 1 (
    echo [ERROR] Gagal mengompilasi resource.rc!
    pause
    exit /b 1
)

echo [2/3] Mengompilasi dan Melink RinganBrowser.exe...
"%COMPILER_DIR%\g++.exe" -std=c++20 -O2 -DUNICODE -D_UNICODE -municode -mwindows ^
    src\main.cpp src\BrowserApp.cpp resource.res ^
    -Isrc -I"webview2_sdk\build\native\include" ^
    -L"webview2_sdk\build\native\x64" -L. ^
    -lWebView2Loader -lgdiplus -lcomctl32 -lole32 -loleaut32 -luuid -lshlwapi ^
    -o RinganBrowser.exe

if errorlevel 1 (
    echo [ERROR] Gagal build RinganBrowser.exe!
    pause
    exit /b 1
)

echo [3/3] Memastikan dependensi runtime...
if not exist "WebView2Loader.dll" (
    copy "webview2_sdk\build\native\x64\WebView2Loader.dll" . >nul
)

echo.
echo ======================================================================
echo  [SUKSES] RinganBrowser.exe berhasil dibuild!
echo  Ukuran file:
dir RinganBrowser.exe | findstr /C:"RinganBrowser.exe"
echo ======================================================================
echo.
pause
