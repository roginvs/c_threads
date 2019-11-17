#!/bin/sh
set -e
#gcc -Wall -o main.out main.c
gcc -o main.out -lpthreads main.c && ./main.out
echo "============================"
echo ""
./main.out