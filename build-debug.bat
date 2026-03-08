@echo off
mkdir build
gcc -O0 -g src/win32_game.c -o build/cgame  -DCGAME_WIN32=1 -DCG_DEVELOPMENT=1 -DCG_PROFILE_FUNCTIONS -std=c99 -municode -Wall -Wconversion -Wpointer-sign -Wextra  -lgdi32 -luser32 -lole32 -luuid
