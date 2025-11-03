: | Command                  | Description                                     |
: | ------------------------ | ----------------------------------------------- |
: | `build` or `build debug` | No optimization, full debug info (`-O0 -g`)     |
: | `build optdebug`         | Optimized but with symbols (`-O2 -g`)           |
: | `build release`          | Fully optimized, no debug info (`-O3 -DNDEBUG`) |

@echo off
setlocal enabledelayedexpansion

:: --- Always start in the script directory
cd /D "%~dp0"

:: --- Create build directory if it doesn't exist
if not exist build mkdir build

:: --- Parse build type argument
set MODE=%1
if "%MODE%"=="" set MODE=debug

:: --- Base compiler options
set CC=clang
set COMMON_FLAGS=-std=c99 -Wall -Wextra -Wshadow -Wpedantic
set DEFINES=

set SRC=code\main.c
set OUT=build\main.exe

:: --- Mode-specific configuration
if /I "%MODE%"=="debug" (
    echo [Mode: DEBUG]
    set CFLAGS=%COMMON_FLAGS% -g -O0
    set DEFINES=-DDEBUG
) else if /I "%MODE%"=="optdebug" (
    echo [Mode: OPTIMIZED DEBUG]
    set CFLAGS=%COMMON_FLAGS% -g -O2
    set DEFINES=-DDEBUG
) else if /I "%MODE%"=="release" (
    echo [Mode: RELEASE]
    set CFLAGS=%COMMON_FLAGS% -O3 -DNDEBUG
    set DEFINES=
) else (
    echo [ERROR] Unknown mode: %MODE%
    echo Usage: build [debug ^| optdebug ^| release]
    exit /b 1
)

:: --- Compile
echo [Compiling %SRC%...]
%CC% %CFLAGS% %DEFINES% -o %OUT% %SRC%
if errorlevel 1 (
    echo [ERROR] Compilation failed.
    exit /b 1
)

echo [Build successful: %OUT%]
