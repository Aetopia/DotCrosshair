@echo off
cd "%~dp0"
gcc.exe -s -municode -O3 -mwindows -nostartfiles -e wWinMain WinMain.c -lOle32 -o DotCrosshair.exe
gcc.exe -s -municode -O3 -shared -nostartfiles -e DllMain DllMain.c -lGdi32 -o DotCrosshair.dll
upx.exe --best --ultra-brute DotCrosshair.dll DotCrosshair.exe>nul 2>&1