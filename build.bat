@echo off
setlocal enabledelayedexpansion

:: --- Always start in the script directory
cd /D "%~dp0"

:: --- Create build directory if it doesn't exist
if not exist build mkdir build

:: --- Compiler flags
set SRC=code\main.c
set OUT=build\main.exe

:: --- Compile
echo [Compiling %SRC%...]
clang -std=c99 -g -o %OUT% %SRC%
if errorlevel 1 (
    echo [ERROR] Compilation failed.
    exit /b 1
)

echo [Build successful: %OUT%]
