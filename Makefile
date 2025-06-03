CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Iinclude -DWIN32_LEAN_AND_MEAN
LIBS = -lraylib -lcomdlg32 -lshell32 -lole32 -lshlwapi -luser32 -lkernel32 -lgdi32 -lwinmm

# Source files
SRCDIR = src
GUIDIR = src/gui
SOURCES = $(wildcard $(SRCDIR)/*.c) $(wildcard $(GUIDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)

# Output
TARGET = AlpenliCloud.exe
OUTDIR = build/output

.PHONY: all clean

all: $(OUTDIR)/$(TARGET)

$(OUTDIR)/$(TARGET): $(OBJECTS) | $(OUTDIR)
	$(CC) $(OBJECTS) -o $@ $(LIBS)
	@echo "Build successful! Executable: $@"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR):
	mkdir -p $(OUTDIR)

clean:
	rm -f $(OBJECTS) $(OUTDIR)/$(TARGET)
	@echo "Cleaned build files"

# Debug target
debug: CFLAGS += -g -DDEBUG
debug: all

# Release target  
release: CFLAGS += -O2 -DNDEBUG
release: all