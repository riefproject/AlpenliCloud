#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <stdbool.h>
#include "raylib.h"
#include "item.h"
#include "component.h"
typedef struct Toolbar
{
    NewButtonProperty newButtonProperty;
    Rectangle currentZeroPosition;
} Toolbar;

void createToolbar(Toolbar *navbar);

void updateToolbar(Toolbar *navbar, Rectangle currentZeroPosition);

void drawToolbar(Toolbar *navbar);

#endif