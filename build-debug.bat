@echo off
mkdir build
gcc -g src/main.c -o build/cgame -municode -lgdi32 -luser32
