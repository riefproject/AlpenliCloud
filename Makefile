# Directories
SRC_DIRS = src src/gui src/data_structure
BUILD_DIR = build/output
BIN_DIR = bin

# Platform detection
UNAME_S := $(shell uname -s)

# Defaults (non-Windows)
EXE_NAME = AlpenliCloud
RESOURCE_RC =
RESOURCE_RES =

# Compiler flags
CFLAGS_BASE = -Iinclude -Iinclude/gui -Iinclude/data_structure
RAYLIB_CFLAGS = -Ilib/raylib/include

ifeq ($(OS),Windows_NT)
	EXE_NAME = AlpenliCloud.exe
	RESOURCE_RC = assets/resource.rc
	RESOURCE_RES = assets/resource.res
	LDFLAGS = lib/raylib/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm
else ifeq ($(UNAME_S),Darwin)
	LDFLAGS = -Llib/raylib/lib -lraylib -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -lm -Wl,-rpath,@executable_path/../lib/raylib/lib
else
	LOCAL_RAYLIB_SO := $(wildcard lib/raylib/lib/libraylib.so)
	PKG_CONFIG := $(shell command -v pkg-config 2>/dev/null)
	ifneq ($(LOCAL_RAYLIB_SO),)
		LDFLAGS = -Llib/raylib/lib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Wl,-rpath,$(CURDIR)/lib/raylib/lib
	else ifneq ($(PKG_CONFIG),)
		PKG_RAYLIB := $(shell pkg-config --exists raylib && echo yes)
		ifeq ($(PKG_RAYLIB),yes)
			RAYLIB_CFLAGS = $(shell pkg-config --cflags raylib)
			LDFLAGS = $(shell pkg-config --libs raylib)
		else
			$(error Raylib tidak ditemukan. Tambahkan lib/raylib/lib/libraylib.so atau install raylib di sistem (contoh: sudo apt install libraylib-dev).)
		endif
	else
		$(error Raylib tidak ditemukan. Tambahkan lib/raylib/lib/libraylib.so atau install raylib di sistem (contoh: sudo apt install libraylib-dev).)
	endif
endif

CFLAGS = $(CFLAGS_BASE) $(RAYLIB_CFLAGS)

EXE_PATH = $(BIN_DIR)/$(EXE_NAME)

# Find all .c files in source directories
SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

# Default target
.PHONY: all clean rebuild
all: $(EXE_PATH)

# Create directories
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo "🔨 Compiling $<..."
	@gcc $(CFLAGS) -c $< -o $@

# Resource compilation (Windows only)
ifneq ($(RESOURCE_RES),)
$(RESOURCE_RES): $(RESOURCE_RC)
	@echo "🎨 Compiling resource file..."
	@windres $< -O coff -o $@
endif

# Linking
$(EXE_PATH): $(OBJS) $(RESOURCE_RES)
	@mkdir -p $(BIN_DIR)
	@echo "🔧 Linking..."
	@gcc $(OBJS) $(RESOURCE_RES) -o $@ $(LDFLAGS)
	@echo "🚀 Build complete!"
	@echo "Run './$(EXE_PATH)' to start AlpenliCloud"

# Clean build files
clean:
	@echo "🧹 Cleaning build directories..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR) $(RESOURCE_RES)

# Rebuild everything
rebuild: clean all
