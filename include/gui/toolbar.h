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
    ButtonWithModalProperty renameButtonProperty;
    Rectangle currentZeroPosition;

    bool isButtonCopyActive;
    bool isButtonCutActive;
    bool isButtonDeleteActive;
    bool isButtonPasteActive;
    bool isbuttonRenameActive;

    Context *ctx;
} Toolbar;

void createToolbar(Toolbar* navbar, Context *ctx);

void updateToolbar(Toolbar* navbar, Context *ctx);

void drawToolbar(Toolbar* navbar);

#endif