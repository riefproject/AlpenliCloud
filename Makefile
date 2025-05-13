CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Ilib/raylib-v5.5/include
LDFLAGS = lib/raylib-v5.5/lib/libraylib.a -lopengl32 -lgdi32 -lwinmm

SRC_DIR = src
BUILD_DIR = build/output/src
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
TARGET = $(BIN_DIR)/Alpendrive.exe

.PHONY: all clean rebuild

all: $(TARGET)
	@echo "🚀 Running Alpendrive..."
	@$(TARGET) || echo "❌ Alpendrive failed to start! Check for errors."

$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "🔧 Linking..."
	@$(CC) $(OBJS) -o $@ $(LDFLAGS) $(RSTFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "🔨 Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(BUILD_DIR):
	@mkdir -p $@

clean:
	@echo "🧹 Cleaning build directories..."
	@rm -rf $(BIN_DIR) build

rebuild: clean all