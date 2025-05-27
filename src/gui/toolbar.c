#include <stdbool.h>
#include <string.h>

#include "gui/component.h"
#include "gui/sidebar.h"
#include "gui/toolbar.h"
#include "gui/ctx.h"

#include "macro.h"
#include "raygui.h"


void createToolbar(Toolbar *toolbar, Context *ctx) {
    toolbar->ctx = ctx;
    toolbar->newButtonProperty = (ButtonWithModalProperty){
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
        .disabled = false
    };
    toolbar->currentZeroPosition = (Rectangle){0};
    toolbar->isButtonCopyActive = false;
    toolbar->isButtonCutActive = false;
    toolbar->isButtonDeleteActive = false;
    toolbar->isButtonPasteActive = false;
}

void updateToolbar(Toolbar *toolbar, Context *ctx) {
    toolbar->ctx = ctx;
    ctx->toolbar = toolbar;

    toolbar->currentZeroPosition = *ctx->currentZeroPosition;
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
        char *dirPath = TextFormat(".dir/%s", ctx->fileManager->currentPath);
        createFile(ctx->fileManager, toolbar->newButtonProperty.selectedType, dirPath, name);
        toolbar->newButtonProperty.itemCreated = false;
    }
    if (toolbar->isButtonCopyActive) {
        copyFile(ctx->fileManager);
        toolbar->isButtonCopyActive = false;
    }
    if (toolbar->isButtonCutActive) {
        cutFile(ctx->fileManager);
        toolbar->isButtonCutActive = false;
    }
    if (toolbar->isButtonDeleteActive) {
        deleteFile(ctx->fileManager);
        toolbar->isButtonDeleteActive = false;
    }
    if (toolbar->isButtonPasteActive) {
        pasteFile(ctx->fileManager);
        toolbar->isButtonPasteActive = false;
    }
}

void drawToolbar(Toolbar *toolbar) {
    float x = toolbar->currentZeroPosition.x;
    float y = toolbar->currentZeroPosition.y;
    float width = toolbar->currentZeroPosition.width;

    int rightStartx = x + width;

    x += toolbar->newButtonProperty.btnRect.width + DEFAULT_PADDING;
    GuiButtonCustom((Rectangle){x, y, 24, 24}, "#22#", "RENAME", false, toolbar->ctx->disableGroundClick);

    x += 24 + DEFAULT_PADDING;
    toolbar->isButtonCutActive = GuiButtonCustom((Rectangle){x, y, 24, 24}, "#17#", "CUT", false, toolbar->ctx->disableGroundClick);

    x += 24 + DEFAULT_PADDING;
    toolbar->isButtonCopyActive = GuiButtonCustom((Rectangle){x, y, 24, 24}, "#16#", "COPY", false, toolbar->ctx->disableGroundClick);

    x += 24 + DEFAULT_PADDING;
    toolbar->isButtonPasteActive = GuiButtonCustom((Rectangle){x, y, 24, 24}, "#18#", "PASTE", false, toolbar->ctx->disableGroundClick);

    rightStartx -= 24;
    toolbar->isButtonDeleteActive = GuiButtonCustom((Rectangle){rightStartx, y, 24, 24}, "#143#", "DELETE", false, toolbar->ctx->disableGroundClick);

    GuiNewButton(&toolbar->newButtonProperty, toolbar->ctx);

    GuiLine((Rectangle){toolbar->currentZeroPosition.x, toolbar->currentZeroPosition.y + 24, toolbar->currentZeroPosition.width, 10}, NULL);
}
