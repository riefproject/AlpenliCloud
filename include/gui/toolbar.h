#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <stdbool.h>
#include "raylib.h"
#include "item.h"
#include "component.h"
#include "file_manager.h"
#include "sidebar.h"
typedef struct Toolbar {
    NewButtonProperty newButtonProperty;
    Rectangle currentZeroPosition;
    FileManager* fileManager; // Pointer to the FileManager to access file operations
    bool isButtonCopyActive;
    bool isButtonCutActive;
    bool isButtonDeleteActive;
    bool isButtonPasteActive;
} Toolbar;

void createToolbar(Toolbar* navbar, FileManager* fileManager, Sidebar* sidebar);

void updateToolbar(Toolbar* navbar, Rectangle currentZeroPosition);

void drawToolbar(Toolbar* navbar);

#endif