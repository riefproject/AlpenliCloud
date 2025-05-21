#!/bin/bash

# Direktori penting
SRC_DIRS=("src" "src/gui")
BUILD_DIR="build/output"
BIN_DIR="bin"
RESOURCE_RES="assets/resource.res"
RESOURCE_RC="assets/resource.rc"
EXE_NAME="AlpenliCloud.exe"

# Flags
CFLAGS="-Iinclude -Ilib/raylib/include"
LDFLAGS="lib/raylib/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm"

object_files=()

clean() {
    echo "🧹 Cleaning build directories..."
    rm -rf "$BUILD_DIR" "$BIN_DIR"
}

compile_if_needed() {
    local src_file=$1
    local out_file=$2

    mkdir -p "$(dirname "$out_file")"

    if [ ! -f "$out_file" ] || [ "$src_file" -nt "$out_file" ]; then
        echo "🔨 Compiling $src_file..."
        gcc $CFLAGS -c "$src_file" -o "$out_file"
        if [ $? -ne 0 ]; then
            echo "❌ Compilation failed: $src_file"
            exit 1
        fi
    else
        echo "✅ Skipping $src_file (up to date)"
    fi
    object_files+=("$out_file")
}

compile_sources() {
    for dir in "${SRC_DIRS[@]}"; do
        for src_file in "$dir"/*.c; do
            [ -f "$src_file" ] || continue
            local filename=$(basename "$src_file" .c)
            local out_file="$BUILD_DIR/$dir/${filename}.o"
            compile_if_needed "$src_file" "$out_file"
        done
    done
}

link_executable() {
    echo "🔧 Linking..."
    windres "$RESOURCE_RC" -O coff -o "$RESOURCE_RES"
    gcc "${object_files[@]}" "$RESOURCE_RES" -o "$BIN_DIR/$EXE_NAME" $LDFLAGS

    if [ $? -ne 0 ]; then
        echo "❌ Linking failed!"
        exit 1
    fi
}

build() {
    echo "📦 Building AlpenliCloud..."
    mkdir -p "$BIN_DIR"

    compile_sources
    link_executable

    echo "🚀 Running AlpenliCloud..."
    sleep 1
    "./$BIN_DIR/$EXE_NAME" || echo "❌ Execution failed!"
}

# Command handler
case "$1" in
    "clean") clean ;;
    "rebuild") clean; build ;;
    ""|"build") build ;;
    *)
        echo "❌ Unknown command: $1"
        echo "Usage: $0 [clean|rebuild|build]"
        exit 1
        ;;
esac
