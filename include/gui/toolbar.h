#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <stdbool.h>
#include "raylib.h"
#include "item.h"
#include "component.h"
#include "file_manager.h"
#include "sidebar.h"

typedef struct Context Context;

typedef struct Toolbar {
    ButtonWithModalProperty newButtonProperty;
    Rectangle currentZeroPosition;
    FileManager* fileManager; // Pointer to the FileManager to access file operations

    bool isButtonCopyActive;
    bool isButtonCutActive;
    bool isButtonDeleteActive;
    bool isButtonPasteActive;

    Context *ctx;
} Toolbar;

void createToolbar(Toolbar* navbar, Context *ctx);

void updateToolbar(Toolbar* navbar, Context *ctx);

void drawToolbar(Toolbar* navbar);

#endif