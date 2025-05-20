# [ERROR]
# Dalam Perbaikan

CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Ilib/raylib/include
LDFLAGS = lib/raylib/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm

SRC_DIR = src
BUILD_DIR = build/output/src
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TARGET = $(BIN_DIR)/AlpenliCloud.exe

.PHONY: all clean rebuild

all: $(TARGET)
	@echo "🚀 Running AlpenliCloud..."
	@$(TARGET) || echo "❌ AlpenliCloud failed to start! Check for errors."

$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "🔧 Linking..."
	windres assets/resource.rc -O coff -o assets/resource.res
	@$(CC) $(OBJS) assets/resource.res -o $@ $(LDFLAGS) $(RSTFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "🔨 Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(BUILD_DIR):
	@if not exist $@ mkdir $@

clean:
	@echo "🧹 Cleaning build directories..."
	@rm -rf $(BIN_DIR) build

rebuild: clean all