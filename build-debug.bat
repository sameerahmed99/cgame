@echo off
mkdir build
gcc -g src/win32_game.c -o build/cgame -municode -lgdi32 -luser32 -lole32
