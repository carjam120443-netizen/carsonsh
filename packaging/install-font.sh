#!/bin/sh
set -eu

FONT_DIR="${HOME}/.local/share/fonts"
FONT_URL="https://github.com/google/fonts/raw/main/ofl/ubuntucondensed/UbuntuCondensed-Regular.ttf"
FONT_FILE="${FONT_DIR}/UbuntuCondensed-Regular.ttf"

mkdir -p "$FONT_DIR"

if [ ! -f "$FONT_FILE" ]; then
    echo "Installing Ubuntu Condensed font..."
    if command -v curl >/dev/null 2>&1; then
        curl -fsSL "$FONT_URL" -o "$FONT_FILE"
    elif command -v wget >/dev/null 2>&1; then
        wget -q "$FONT_URL" -O "$FONT_FILE"
    else
        echo "CarsonSH: curl or wget is required to install Ubuntu Condensed." >&2
        exit 1
    fi
    echo "Installed Ubuntu Condensed to $FONT_FILE"
else
    echo "Ubuntu Condensed is already installed."
fi

if command -v fc-cache >/dev/null 2>&1; then
    fc-cache -f "$FONT_DIR" >/dev/null 2>&1 || true
fi

echo "Set your terminal emulator's font to 'Ubuntu Condensed' to use it."
