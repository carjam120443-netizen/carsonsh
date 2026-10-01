#!/bin/sh
set -eu
PREFIX="${PREFIX:-$HOME/.local}"
BINDIR="$PREFIX/bin"
[ -f ./build/carsonsh ] || ./build.sh
mkdir -p "$BINDIR"
cp ./build/carsonsh "$BINDIR/carsonsh"
chmod +x "$BINDIR/carsonsh"
echo "Installed CarsonSH to $BINDIR/carsonsh"
echo "Make sure $BINDIR is in your PATH."
