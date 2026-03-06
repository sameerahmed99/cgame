@echo off
mkdir build
gcc src/win32_game.c -o build/cgame  -DCGAME_WIN32=1 -std=c99 -municode -lgdi32 -luser32 -lole32 -luuid
