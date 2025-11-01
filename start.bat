@echo off

:: Configura las variables de entorno de Visual Studio
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

:: Lanza Zed Editor
start "" "C:\Users\strange\AppData\Local\Programs\Zed\Zed.exe"

:: Lanza RAD Debugger
start "" "D:\toolbox\5 - dev\raddbg.exe"
