#include <string.h>
#include <stdbool.h>

#include "macro.h"
#include "gui/component.h"
#include "gui/toolbar.h"
#include "raygui.h"

void createToolbar(Toolbar *toolbar)
{
    toolbar->currentZeroPosition = (Rectangle){0};
}

void updateToolbar(Toolbar *toolbar, Rectangle currentZeroPosition)
{
    toolbar->currentZeroPosition = currentZeroPosition;
    toolbar->currentZeroPosition.y += 24 + DEFAULT_PADDING;
}

void drawToolbar(Toolbar *toolbar)
{
    GuiButtonCustom((Rectangle){toolbar->currentZeroPosition.x, toolbar->currentZeroPosition.y, 84, 24}, "#65# NEW", "CREATE NEW ITEM", false);

    // Tombol "PILIH BANYAK"
    GuiButtonCustom(
        (Rectangle){toolbar->currentZeroPosition.x + toolbar->currentZeroPosition.width - 130, toolbar->currentZeroPosition.y, 130, 24},
        "#112# PILIH BANYAK",
        "RENAME", true);

    // Tombol lainnya mundur dari tombol "PILIH BANYAK"
    float rightStartX = toolbar->currentZeroPosition.x + toolbar->currentZeroPosition.width - 130 - DEFAULT_PADDING;

    rightStartX -= 24; 
    GuiButtonCustom((Rectangle){rightStartX, toolbar->currentZeroPosition.y, 24, 24}, "#22#", "RENAME", true);

    rightStartX -= 24 + DEFAULT_PADDING; 
    GuiButtonCustom((Rectangle){rightStartX, toolbar->currentZeroPosition.y, 24, 24}, "#18#", "PASTE", true);

    rightStartX -= 24 + DEFAULT_PADDING; 
    GuiButtonCustom((Rectangle){rightStartX, toolbar->currentZeroPosition.y, 24, 24}, "#16#", "COPY", true);

    rightStartX -= 24 + DEFAULT_PADDING; 
    GuiButtonCustom((Rectangle){rightStartX, toolbar->currentZeroPosition.y, 24, 24}, "#17#", "CUT", true);

    GuiLine((Rectangle){toolbar->currentZeroPosition.x, toolbar->currentZeroPosition.y + 24, toolbar->currentZeroPosition.width, 10}, NULL);
}
