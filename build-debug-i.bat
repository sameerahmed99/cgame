@echo off
mkdir build
gcc -g src/win32_game.c -E intermediate/cgame.i  -DCGAME_WIN32=1 -DCG_DEVELOPMENT=1 -std=c99 -municode -Wall -Wconversion -Wpointer-sign -Wextra -lgdi32 -luser32 -lole32 -luuid
