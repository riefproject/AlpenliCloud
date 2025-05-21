#include <string.h>
#include <stdbool.h>

#include "macro.h"
#include "gui/component.h"
#include "gui/toolbar.h"
#include "raygui.h"


void createToolbar(Toolbar *toolbar) {
    toolbar->currentZeroPosition = (Rectangle) {0};
}

void updateToolbar(Toolbar *toolbar, Rectangle currentZeroPosition) {
    toolbar->currentZeroPosition = currentZeroPosition;
    toolbar->currentZeroPosition.y += 24 + DEFAULT_PADDING;
}

void drawToolbar(Toolbar *toolbar) {
    GuiButtonTooltip((Rectangle){toolbar->currentZeroPosition.x, toolbar->currentZeroPosition.y, 84, 24}, "#65# NEW", "CREATE NEW ITEM");
    GuiButtonTooltip((Rectangle){toolbar->currentZeroPosition.x + 84 + DEFAULT_PADDING, toolbar->currentZeroPosition.y, 24, 24}, "#17#", "CUT");
    GuiButtonTooltip((Rectangle){toolbar->currentZeroPosition.x + 84 + 24 + DEFAULT_PADDING * 2, toolbar->currentZeroPosition.y, 24, 24}, "#16#", "COPY");
    GuiButtonTooltip((Rectangle){toolbar->currentZeroPosition.x + 84 + 24 * 2 + DEFAULT_PADDING * 3, toolbar->currentZeroPosition.y, 24, 24}, "#18#", "PASTE");
    GuiButtonTooltip((Rectangle){toolbar->currentZeroPosition.x + 84 + 24 * 3 + DEFAULT_PADDING * 4, toolbar->currentZeroPosition.y, 24, 24}, "#22#", "RENAME");

    GuiLine((Rectangle){toolbar->currentZeroPosition.x, toolbar->currentZeroPosition.y + 24, toolbar->currentZeroPosition.width, 10}, NULL);
}
