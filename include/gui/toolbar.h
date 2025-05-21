#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <stdbool.h>
#include "raylib.h"
typedef struct Toolbar {
    Rectangle currentZeroPosition;
} Toolbar;

void createToolbar(Toolbar *navbar);

void updateToolbar(Toolbar *navbar, Rectangle currentZeroPosition);

void drawToolbar(Toolbar *navbar);

#endif