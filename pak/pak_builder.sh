#!/bin/bash
set -e

# Пути
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LUA_DIR="$SCRIPT_DIR/lua"
OUTPUT_DIR="$SCRIPT_DIR/output"
PAK_NAME="ModMenu_P.pak"
REPAK="$SCRIPT_DIR/repak"

# Очистка
rm -rf "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"

# Проверка наличия repak
if [ ! -f "$REPAK" ]; then
    echo "repak не найден. Скачай с https://github.com/trumank/repak/releases"
    exit 1
fi

# Создаём структуру для pak
TEMP_DIR="$SCRIPT_DIR/temp_pak"
rm -rf "$TEMP_DIR"
mkdir -p "$TEMP_DIR/ShadowTrackerExtra/Content/Lua/Mods"

# Копируем Lua-файлы
cp "$LUA_DIR"/*.lua "$TEMP_DIR/ShadowTrackerExtra/Content/Lua/Mods/"

# Собираем pak
"$REPAK" pack "$TEMP_DIR" "$OUTPUT_DIR/$PAK_NAME"

echo "Pak собран: $OUTPUT_DIR/$PAK_NAME"
