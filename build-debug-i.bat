@echo off
mkdir build
gcc -g src/cgame.c -E intermediate/cgame.i  -DCGAME_WIN32=1 -DCGAME_DEVELOPMENT=1 -std=c99 -municode -Wall -Wconversion -Wpointer-sign -Wextra -lgdi32 -luser32 -lole32 -luuid
