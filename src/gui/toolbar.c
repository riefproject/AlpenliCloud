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
    float x = toolbar->currentZeroPosition.x;
    float y = toolbar->currentZeroPosition.y;
    float width = toolbar->currentZeroPosition.width;

    
    GuiButtonCustom((Rectangle){x, y, 84, 24}, "#65# NEW", "CREATE NEW ITEM", false);

    
    GuiButtonCustom((Rectangle){x + 84 + DEFAULT_PADDING, y, 130, 24}, "#112# PILIH BANYAK", "MULTI SELECT", true);

    
    float rightStartX = x + width;

    rightStartX -= 24; 
    GuiButtonCustom((Rectangle){rightStartX, y, 24, 24}, "#22#", "RENAME", true);

    rightStartX -= 24 + DEFAULT_PADDING; 
    GuiButtonCustom((Rectangle){rightStartX, y, 24, 24}, "#18#", "PASTE", true);

    rightStartX -= 24 + DEFAULT_PADDING; 
    GuiButtonCustom((Rectangle){rightStartX, y, 24, 24}, "#16#", "COPY", true);

    rightStartX -= 24 + DEFAULT_PADDING; 
    GuiButtonCustom((Rectangle){rightStartX, y, 24, 24}, "#17#", "CUT", true);

    GuiLine((Rectangle){toolbar->currentZeroPosition.x, toolbar->currentZeroPosition.y + 24, toolbar->currentZeroPosition.width, 10}, NULL);
}
