@echo off
cd "%~dp0"
gcc.exe -municode -O3 -mwindows -s WinMain.c -lOle32 -o DotCrosshair.exe
gcc.exe -municode -O3 -shared -s DllMain.c -lGdi32 -o DotCrosshair.dll
upx.exe --best --ultra-brute DotCrosshair.dll DotCrosshair.exe>nul 2>&1