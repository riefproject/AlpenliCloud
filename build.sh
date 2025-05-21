#!/bin/bash

SRC_DIRS=("src" "src/gui")
BUILD_DIR="build/output"
BIN_DIR="bin"
EXE_NAME="AlpenliCloud.exe"
EXE_PATH="$BIN_DIR/$EXE_NAME"
RESOURCE_RC="assets/resource.rc"
RESOURCE_RES="assets/resource.res"

CFLAGS="-Iinclude -Ilib/raylib/include"
LDFLAGS="lib/raylib/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm"

object_files=()

clean() {
    echo "🧹 Cleaning build directories..."
    rm -rf "$BUILD_DIR" "$BIN_DIR" "$RESOURCE_RES"
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

compile_resource_if_needed() {
    if [ ! -f "$RESOURCE_RES" ] || [ "$RESOURCE_RC" -nt "$RESOURCE_RES" ]; then
        echo "🎨 Compiling resource file..."
        windres "$RESOURCE_RC" -O coff -o "$RESOURCE_RES"
        if [ $? -ne 0 ]; then
            echo "❌ Resource compilation failed!"
            exit 1
        fi
    else
        echo "✅ Skipping resource compilation (up to date)"
    fi
}

link_if_needed() {
    compile_resource_if_needed

    local need_link=false
    if [ ! -f "$EXE_PATH" ] || [ "$RESOURCE_RES" -nt "$EXE_PATH" ]; then
        need_link=true
    else
        for obj in "${object_files[@]}"; do
            if [ "$obj" -nt "$EXE_PATH" ]; then
                need_link=true
                break
            fi
        done
    fi

    if $need_link; then
        echo "🔧 Linking..."
        gcc "${object_files[@]}" "$RESOURCE_RES" -o "$EXE_PATH" $LDFLAGS
        if [ $? -ne 0 ]; then
            echo "❌ Linking failed!"
            exit 1
        fi
    else
        echo "✅ Skipping linking (up to date)"
    fi
}

build() {
    echo "🚧 Starting build process..."
    mkdir -p "$BIN_DIR"
    compile_sources
    link_if_needed
    echo "🚀 Running AlpenliCloud..."
    sleep 1
    "./$EXE_PATH" || echo "❌ AlpenliCloud failed to start! Check for errors."
}

case "$1" in
    "clean") clean ;;
    "rebuild") clean; build ;;
    ""|"build") build ;;
    *)
        echo "❌ Unknown parameter: $1"
        echo "Usage: $0 [clean|rebuild|build]"
        exit 1
        ;;
esac
