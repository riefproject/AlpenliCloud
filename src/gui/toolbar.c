#include <stdbool.h>
#include <string.h>

#include "gui/component.h"
#include "gui/toolbar.h"
#include "macro.h"
#include "raygui.h"
#include "gui/sidebar.h"

void createToolbar(Toolbar *toolbar, FileManager *fileManager, Sidebar *sidebar) {

    toolbar->newButtonProperty = (NewButtonProperty){
        .btnRect = {50, 50, 100, 24},
        .dropdownRect = {50, 82, 100, 60},
        .modalRect = {300, 200, 300, 150},
        .placeholder = "#65# New",
        .tooltip = "Add a file or folder",
        .inputBuffer = "",
        .dropdownIndex = 0,
        .dropdownActive = false,
        .selectedType = ITEM_FILE,
        .showModal = false,
        .itemCreated = false,
        .inputEditMode = false,
        .disabled = false,
        .sidebar = sidebar
    };
    toolbar->currentZeroPosition = (Rectangle){0};
    toolbar->fileManager = fileManager;
}

void updateToolbar(Toolbar *toolbar, Rectangle currentZeroPosition) {
    toolbar->currentZeroPosition = currentZeroPosition;
    toolbar->currentZeroPosition.y += 24 + DEFAULT_PADDING;

    toolbar->newButtonProperty.btnRect.x = toolbar->currentZeroPosition.x;
    toolbar->newButtonProperty.btnRect.y = toolbar->currentZeroPosition.y;

    toolbar->newButtonProperty.dropdownRect.x = toolbar->newButtonProperty.btnRect.x;
    toolbar->newButtonProperty.dropdownRect.y = toolbar->newButtonProperty.btnRect.y + toolbar->newButtonProperty.btnRect.height + 2;

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int modalWidth = toolbar->newButtonProperty.modalRect.width;
    int modalHeight = toolbar->newButtonProperty.modalRect.height;

    toolbar->newButtonProperty.modalRect.x = (screenWidth - modalWidth) / 2;
    toolbar->newButtonProperty.modalRect.y = (screenHeight - modalHeight) / 2;

    if (toolbar->newButtonProperty.itemCreated) {
        char *name = toolbar->newButtonProperty.inputBuffer;
        createFile(toolbar->fileManager, toolbar->newButtonProperty.selectedType, name);
        toolbar->newButtonProperty.itemCreated = false;
    }
}

void drawToolbar(Toolbar *toolbar) {
    float x = toolbar->currentZeroPosition.x;
    float y = toolbar->currentZeroPosition.y;
    float width = toolbar->currentZeroPosition.width;

    GuiButtonCustom((Rectangle){x + toolbar->newButtonProperty.btnRect.width + DEFAULT_PADDING, y, 130, 24}, "#112# PILIH BANYAK", "MULTI SELECT", true);

    int rightStartx = x + width;

    x += toolbar->newButtonProperty.btnRect.width + 130;
    x += 24;
    GuiButtonCustom((Rectangle){x, y, 24, 24}, "#22#", "RENAME", true);

    x += 24 + DEFAULT_PADDING;
    GuiButtonCustom((Rectangle){x, y, 24, 24}, "#18#", "PASTE", true);

    x += 24 + DEFAULT_PADDING;
    GuiButtonCustom((Rectangle){x, y, 24, 24}, "#16#", "COPY", true);

    x += 24 + DEFAULT_PADDING;
    GuiButtonCustom((Rectangle){x, y, 24, 24}, "#17#", "CUT", true);

    rightStartx -= 24;
    GuiButtonCustom((Rectangle){rightStartx, y, 24, 24}, "#143#", "DELETE", true);

    GuiNewButton(&toolbar->newButtonProperty);

    GuiLine((Rectangle){toolbar->currentZeroPosition.x, toolbar->currentZeroPosition.y + 24, toolbar->currentZeroPosition.width, 10}, NULL);
}
