#include <string.h>
#include <stdbool.h>

#include "macro.h"
#include "gui/component.h"
#include "gui/navbar.h"
#include "raygui.h"


void createNavbar(Navbar *navbar)
{
    navbar->textboxPatheditMode = false;
    strcpy(navbar->textboxPath, "") ;

    navbar->textboxSearcheditMode = false;
    strcpy(navbar->textboxSearch, "#42# Search Item");
    
    navbar->currentZeroPosition = (Rectangle) {0};
}

void updateNavbar(Navbar *navbar, Rectangle currentZeroPosition)
{
    navbar->currentZeroPosition = currentZeroPosition;
}

void drawNavbar(Navbar *navbar)
{
    GuiButtonTooltip((Rectangle){navbar->currentZeroPosition.x, navbar->currentZeroPosition.y, 24, 24}, "#56#", "UNDO");
    GuiButtonTooltip((Rectangle){navbar->currentZeroPosition.x + 24 + DEFAULT_PADDING, navbar->currentZeroPosition.y, 24, 24}, "#57#", "REDO");
    GuiButtonTooltip((Rectangle){navbar->currentZeroPosition.x + 24 * 2 + DEFAULT_PADDING * 2, navbar->currentZeroPosition.y, 24, 24}, "#117#", "BACK");

    if (GuiTextBox((Rectangle){navbar->currentZeroPosition.x + 24 * 3 + DEFAULT_PADDING * 3, navbar->currentZeroPosition.y, 500, 24}, navbar->textboxPath, 1024, navbar->textboxPatheditMode))
        navbar->textboxPatheditMode = !navbar->textboxPatheditMode;

    if (GuiTextBox((Rectangle){navbar->currentZeroPosition.x + 500 + 24 * 3 + DEFAULT_PADDING * 4, navbar->currentZeroPosition.y, 170, 24}, navbar->textboxSearch, 1024, navbar->textboxSearcheditMode))
        navbar->textboxSearcheditMode = !navbar->textboxSearcheditMode;

    GuiLine((Rectangle){navbar->currentZeroPosition.x, navbar->currentZeroPosition.y + 24, navbar->currentZeroPosition.width, 10}, NULL);
}
