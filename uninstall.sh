#!/bin/sh
set -eu
TARGET="${PREFIX:-$HOME/.local}/bin/carsonsh"
if [ -f "$TARGET" ]; then
    rm "$TARGET"
    echo "Removed $TARGET"
else
    echo "CarsonSH is not installed at $TARGET"
fi
