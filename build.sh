#!/bin/sh
set -eu
CC="${CC:-cc}"
CFLAGS="${CFLAGS:--std=c11 -Wall -Wextra -Wpedantic -O2}"
mkdir -p build
echo "Building CarsonSH..."
"$CC" $CFLAGS -Isrc src/main.c src/builtins.c -o build/carsonsh
chmod +x build/carsonsh
echo "Built build/carsonsh"
