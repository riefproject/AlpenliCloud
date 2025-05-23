#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "macro.h"
#include "file_manager.h"
#include "gui/component.h"
#include "gui/navbar.h"
#include "raygui.h"

void createNavbar(Navbar *navbar)
{
    navbar->textboxPatheditMode = false;
    strcpy(navbar->textboxPath, "");

    navbar->textboxSearcheditMode = false;
    strcpy(navbar->textboxSearch, "");

    navbar->currentZeroPosition = (Rectangle){0};
}

void updateNavbar(Navbar *navbar, Rectangle currentZeroPosition, FileManager *fileManager)
{
    navbar->fileManager = fileManager;
    navbar->currentZeroPosition = currentZeroPosition;
}

void drawNavbar(Navbar *navbar)
{
    float x = navbar->currentZeroPosition.x;
    float y = navbar->currentZeroPosition.y;
    float totalWidth = navbar->currentZeroPosition.width;

    float buttonSize = 24;
    float spacing = DEFAULT_PADDING;
    float searchBoxWidth = 170;
    float searchBoxX = x + totalWidth - searchBoxWidth;

    float buttonsTotalWidth = buttonSize * 3 + spacing * 2;

    float pathBoxStartX = x + buttonsTotalWidth + spacing;
    float pathBoxMaxWidth = searchBoxX - pathBoxStartX - spacing;
    float pathBoxWidth = (pathBoxMaxWidth >= 170) ? pathBoxMaxWidth : 170;

    searchBoxX = searchBoxX < pathBoxStartX + pathBoxWidth + spacing ? pathBoxStartX + pathBoxWidth + spacing : searchBoxX;

    GuiButtonCustom((Rectangle){x, y, buttonSize, buttonSize}, "#56#", "UNDO", false);
    GuiButtonCustom((Rectangle){x + buttonSize + spacing, y, buttonSize, buttonSize}, "#57#", "REDO", false);
    if (GuiButtonCustom((Rectangle){x + (buttonSize + spacing) * 2, y, buttonSize, buttonSize}, "#117#", "BACK", false))
        goBack(navbar->fileManager);

    GuiTextBoxCustom((Rectangle){pathBoxStartX, y, pathBoxWidth, buttonSize}, "#1#", navbar->textboxPath, 1024, &navbar->textboxPatheditMode, false);
    GuiTextBoxCustom((Rectangle){searchBoxX, y, searchBoxWidth, buttonSize}, "#42# Search Item", navbar->textboxSearch, 1024, &navbar->textboxSearcheditMode, false);

    GuiLine((Rectangle){x, y + buttonSize, totalWidth, 10}, NULL);
}
