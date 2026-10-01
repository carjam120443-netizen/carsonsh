#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
CONFIG_DIR="${HOME}/.config/carsonsh"
THEME_DIR="${CONFIG_DIR}/themes"
PLUGIN_DIR="${CONFIG_DIR}/plugins"

mkdir -p "$THEME_DIR" "$PLUGIN_DIR"
cp "$ROOT/themes/"*.conf "$THEME_DIR/"
cp "$ROOT/plugins/"*.conf "$PLUGIN_DIR/"

CONFIG="${CONFIG_DIR}/config"
touch "$CONFIG"

add_config() {
    entry="$1"
    if ! grep -Fqx "$entry" "$CONFIG"; then
        printf '%s\n' "$entry" >> "$CONFIG"
    fi
}

add_config "theme carson-green"
add_config "plugin git"
add_config "plugin linux"
add_config "plugin shortcuts"

echo "CarsonSH Framework installed to $CONFIG_DIR"
echo "Restart CarsonSH or run: reload"
