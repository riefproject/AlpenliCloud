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
        -Ilib/raylib/include
        -g"
WINFLAGS="-lcomdlg32
         -lshell32
         -lole32
         -lshlwapi
         -lkernel32"
LDFLAGS="lib/raylib/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm"
ALLFLAGS="$WINFLAGS $LDFLAGS"
OBJECT_FILES=()

# Biar CLI nya cakep
RED='\e[31m'
GREEN='\e[32m'
YELLOW='\e[33m'
BLUE='\e[34m'
RESET='\e[0m' # No Color
BOLD='\e[1m'
ITALIC='\e[3m'
UNDERLINE='\e[4m'

# =====================================================================================
# . . . FUNCTIONS . . .
# =====================================================================================

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
        clang $CFLAGS -c "$src_file" -o "$out_file"
        if [ $? -ne 0 ]; then
            echo -e "${RED}❌ Compilation failed: $src_file ${RESET}"
            exit 1
        fi
    else
        echo -e "${GREEN}✅ Skipping $src_file (up to date) ${RESET}"
    fi

    OBJECT_FILES+=("$out_file")
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
        for obj in "${OBJECT_FILES[@]}"; do
            if [ "$obj" -nt "$EXE_PATH" ]; then
                need_link=true
                break
            fi
        done
    fi

    if $need_link; then
        echo "🔧 Linking..."
        clang -g "${OBJECT_FILES[@]}" "$RESOURCE_RES" -o "$EXE_PATH" $ALLFLAGS
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
        # clear
        echo -e "\n\nMantap! Trims sudah pakai ${BLUE}AlpenliCloud${RESET}\n\n"
    fi
}

parse_args() {
    DEBUG=0
    HELP=0
    POSITIONAL=()

    for arg in "$@"; do
        if [ "$arg" == "--debug" ]; then
            DEBUG=1
        elif [ "$arg" == "--help" ]; then
            HELP=1
        else
            POSITIONAL+=("$arg")
        fi
    done

    if [ $HELP -eq 1 ]; then
        if [ ${#POSITIONAL[@]} -ne 0 ] || [ $DEBUG -eq 1 ]; then
            echo "❌ Error: --help harus dipanggil tanpa parameter lain."
            show_help
            exit 1
        fi
        show_help
        exit 0
    fi

    # Set global variabel untuk dipakai di main
    DEBUG_FLAG=$DEBUG
    set -- "${POSITIONAL[@]}"
    POSITIONAL_ARGS=("$@")
}

show_help() {
    echo -e "\n${BOLD}${BLUE}AlpenliCloud${RESET}${BOLD} Build Script${RESET}"
    echo -e "============================================================\n"
    
    echo -e "Usage: $0 [--debug] [clean|build|rebuild] [--help]\n"
    echo -e "${UNDERLINE}Commands:${RESET}"
    echo -e "  ${BOLD}clean    ${RESET}  : Bersihkan hasil build"
    echo -e "  ${BOLD}build    ${RESET}  : Compile dan link (incremental)"
    echo -e "  ${BOLD}rebuild  ${RESET}  : Bersihkan lalu build ulang"
    echo -e "  ${BOLD}--debug  ${RESET}  : Aktifkan mode debug (set -x)"
    echo -e "  ${BOLD}--help   ${RESET}  : Tampilkan pesan ini\n"
    echo -e "${UNDERLINE}Notes:${RESET}"
    echo -e "  --help harus dipanggil sendiri, tanpa param lain."
}

# =====================================================================================
# . . . MAIN SCRIPT . . .
# =====================================================================================

parse_args "$@"

if [ "$DEBUG_FLAG" -eq 1 ]; then
    set -x
fi

# Lanjut dengan main case pakai "${POSITIONAL_ARGS[0]}"
case "${POSITIONAL_ARGS[0]}" in
    "clean") clean ;;
    "rebuild") clean; build ;;
    ""|"build"|"run") build ;;
    *)
        echo -e "${RED}❌ Unknown parameter: ${POSITIONAL_ARGS[0]} ${RESET}"
        show_help
        exit 1
        ;;
esac
