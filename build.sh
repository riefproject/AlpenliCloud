#!/bin/bash

# pake array karna akan diiterasi. Kalo mau nambah subfolder tambahin aja
SRC_DIRS=("src" 
          "src/gui" 
          "src/data_structure")

BUILD_DIR="build/output"
BIN_DIR="bin"
EXE_NAME="AlpenliCloud.exe"
EXE_PATH="$BIN_DIR/$EXE_NAME"
RESOURCE_RC="assets/resource.rc"
RESOURCE_RES="assets/resource.res"

# bukan array karna ga akan diiterasi (cuma flag). Tambahin aja kalo butuh subfolder tambahan
WNO="-Wno-discarded-qualifiers
     -Wno-unused-parameter
     -Wno-unused-variable"

CFLAGS="$WNO
        -Iinclude 
        -Iinclude/gui
        -Iinclude/data_structure
        -Ilib/raylib/include"
LDFLAGS="lib/raylib/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm"
object_files=()

# Biar CLI nya cakep
RED='\e[31m'
GREEN='\e[32m'
YELLOW='\e[33m'
BLUE='\e[34m'
RESET='\e[0m' # No Color

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
            echo -e "${RED}❌ Compilation failed: $src_file ${RESET}"
            exit 1
        fi
    else
        echo -e "${GREEN}✅ Skipping $src_file (up to date) ${RESET}"
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
            echo -e "${RED}❌ Resource compilation failed! ${RESET}"
            exit 1
        fi
    else
        echo -e "${GREEN}✅ Skipping resource compilation (up to date) ${RESET}"
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
            echo -e "${RED}❌ Linking failed! ${RESET}"
            exit 1
        fi
    else
        echo -e "${GREEN}✅ Skipping linking (up to date) ${RESET}"
    fi
}

build() {
    echo "🚧 Starting build process..."
    mkdir -p "$BIN_DIR"
    compile_sources
    link_if_needed

    echo -e "\n🚀 Running ${BLUE}AlpenliCloud...${RESET}"
    sleep 1
    "./$EXE_PATH" || echo -e "${RED}❌ AlpenliCloud failed to start! Check for errors. ${RESET}"
    if [ $? -eq 0 ]; then
        clear
        echo -e "\n\nMantap! Trims sudah pakai ${BLUE}AlpenliCloud${RESET}\n\n"
    fi
}

case "$1" in
    "clean") clean ;;
    "rebuild") clean; build ;;
    ""|"build") build ;;
    *)
        echo -e "${RED}❌ Unknown parameter: $1 ${RESET}"
        echo "Usage: $0 [clean|rebuild|build]"
        exit 1
        ;;
esac
