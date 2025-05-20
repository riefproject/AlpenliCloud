#!/bin/bash

clean() {
    echo "🧹 Cleaning build directories..."
    rm -rf bin build
}

build() {
    echo "Compiling Aplendrive..."
    mkdir -p bin build/output/src build/output/library

    CFLAGS="-Wall -Wextra -Iinclude -Ilib/raylib/include "
    LDFLAGS="lib/raylib/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm"

    object_files=""
    for src_file in src/main.c; do
        if [ -f "$src_file" ]; then
            filename=$(basename "$src_file")
            object_file="build/output/src/${filename%.c}.o"
            compile "$src_file" "$object_file"
            object_files="$object_files $object_file"
        fi
    done

    echo "🔧 Linking..."
    windres assets/resource.rc -O coff -o assets/resource.res
    gcc $object_files assets/resource.res -o bin/AlpenliCloud.exe $LDFLAGS $RSTFLAGS

    if [ $? -ne 0 ]; then
        echo "❌ Linking failed!"
        exit 1
    fi

    clear
    echo "🚀 Running AlpenliCloud..."
    sleep 1
    ./bin/AlpenliCloud.exe || echo "❌ AlpenliCloud failed to start! Check for errors."
}

compile() {
    local src_file=$1
    local out_file=$2

    echo "🔨 Compiling $src_file..."
    gcc $CFLAGS -c "$src_file" -o "$out_file"

    if [ $? -ne 0 ]; then
        echo "❌ Compilation of $src_file failed!"
        exit 1
    fi
}

case "$1" in
    "clean")
        clean
        ;;
    "rebuild")
        clean
        build
        ;;
    "")
        build
        ;;
    *)
        echo "❌ Unknown parameter: $1"
        echo "Usage: $0 [clean|rebuild]"
        exit 1
        ;;
esac