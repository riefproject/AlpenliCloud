#include <stdbool.h>
#include <string.h>

#include "ctx.h"
#include "gui/component.h"
#include "gui/sidebar.h"
#include "gui/toolbar.h"

#include "macro.h"
#include "raygui.h"

void createToolbar(Toolbar *toolbar, Context *ctx) {
    toolbar->ctx = ctx;
    toolbar->newButtonProperty = (ButtonWithModalProperty){
        .btnRect = {50, 50, 100, 24},
        .dropdownRect = {50, 82, 100, 60},
        .modalRect = {300, 200, 300, 150},
        .yesButtonText = "CREATE",
        .noButtonText = "CANCEL",
        .title = "Create Item",
        .placeholder = "#65# New",
        .tooltip = "Add a file or folder",
        .inputBuffer = "",
        .dropdownIndex = 0,
        .dropdownActive = false,
        .selectedType = ITEM_FILE,
        .showModal = false,
        .itemCreated = false,
        .inputEditMode = false,
        .disabled = false};
    toolbar->renameButtonProperty = (ButtonWithModalProperty){
        .btnRect = {0},
        .dropdownRect = {0},
        .modalRect = {300, 200, 300, 150},
        .noButtonText = "CANCEL",
        .yesButtonText = "RENAME",
        .title = "Rename Item",
        .placeholder = NULL,
        .tooltip = "NULL",
        .inputBuffer = "",
        .dropdownIndex = 0,
        .dropdownActive = false,
        .selectedType = ITEM_FILE,
        .showModal = false,
        .itemCreated = false,
        .inputEditMode = false,
        .disabled = false};
    toolbar->currentZeroPosition = (Rectangle){0};
    toolbar->isButtonCopyClicked = false;
    toolbar->isButtonCutClicked = false;
    toolbar->isButtonDeleteClicked = false;
    toolbar->isButtonPasteClicked = false;
    toolbar->isbuttonRenameClicked = false;
    toolbar->isButtonRestoreClicked = false;
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
        createFile(ctx->fileManager, toolbar->newButtonProperty.selectedType, dirPath, name, true);
        toolbar->newButtonProperty.itemCreated = false;
    }
    if (toolbar->isButtonCopyClicked) {
        copyFile(ctx->fileManager);
        toolbar->isButtonCopyClicked = false;
    }
    if (toolbar->isButtonCutClicked) {
        cutFile(ctx->fileManager);
        toolbar->isButtonCutClicked = false;
    }
    if (toolbar->isButtonDeleteClicked) {
        deleteFile(ctx->fileManager, true);
        toolbar->isButtonDeleteClicked = false;
    }
    if (toolbar->isButtonPasteClicked) {
        pasteFile(ctx->fileManager);
        toolbar->isButtonPasteClicked = false;
    }
    if (toolbar->isbuttonRenameClicked) {
        toolbar->renameButtonProperty.showModal = true;
        toolbar->renameButtonProperty.inputEditMode = true;
        Item *selectedItem = ctx->fileManager->selectedItem.head ? (Item *)ctx->fileManager->selectedItem.head->data : NULL;
        if (selectedItem) {
            strncpy(toolbar->renameButtonProperty.inputBuffer, selectedItem->name, MAX_STRING_LENGTH - 1);
            toolbar->renameButtonProperty.inputBuffer[MAX_STRING_LENGTH - 1] = '\0';
            toolbar->renameButtonProperty.selectedType = selectedItem->type;
        } else {
            toolbar->renameButtonProperty.inputBuffer[0] = '\0';
        }
    }

    if (toolbar->renameButtonProperty.itemCreated) {
        char *newName = toolbar->renameButtonProperty.inputBuffer;
        char *name = toolbar->ctx->fileManager->selectedItem.head ? ((Item *)toolbar->ctx->fileManager->selectedItem.head->data)->name : NULL;
        if (name == NULL || strlen(name) == 0) {
            printf("[ERROR] No item selected for renaming.\n");
            return;
        }

        char *filePath = TextFormat(".dir/%s/%s", ctx->fileManager->currentPath, name);

        printf("filePath: %s\n", filePath);

        renameFile(ctx->fileManager, filePath, newName, true);
        toolbar->renameButtonProperty.showModal = false;
        toolbar->renameButtonProperty.inputEditMode = false;
        toolbar->renameButtonProperty.inputBuffer[0] = '\0';
        toolbar->renameButtonProperty.itemCreated = false;
        toolbar->isbuttonRenameClicked = false;
    }

    if (toolbar->isButtonRestoreClicked) {
        toolbar->isButtonRestoreClicked = false;
        // recoverFile(ctx->fileManager);
    }
}

void drawToolbar(Toolbar *toolbar) {
    float x = toolbar->currentZeroPosition.x;
    float y = toolbar->currentZeroPosition.y;
    float width = toolbar->currentZeroPosition.width;

    int rightStartx = x + width;
    int selectedItemCount = get_length(toolbar->ctx->fileManager->selectedItem);

    // printf("[LOG] Selected Item Count: %d, boolean: %d\n", selectedItemCount, selectedItemCount <= 0);

    if (!toolbar->ctx->fileManager->isRootTrash) {
        x += toolbar->newButtonProperty.btnRect.width + DEFAULT_PADDING;
        toolbar->isbuttonRenameClicked = GuiButtonCustom((Rectangle){x, y, 24, 24}, "#22#", "RENAME", (selectedItemCount <= 0 || selectedItemCount > 1), toolbar->ctx->disableGroundClick);

        x += 24 + DEFAULT_PADDING;
        toolbar->isButtonCutClicked = GuiButtonCustom((Rectangle){x, y, 24, 24}, "#17#", "CUT", selectedItemCount <= 0, toolbar->ctx->disableGroundClick);

        x += 24 + DEFAULT_PADDING;
        toolbar->isButtonCopyClicked = GuiButtonCustom((Rectangle){x, y, 24, 24}, "#16#", "COPY", selectedItemCount <= 0, toolbar->ctx->disableGroundClick);

        x += 24 + DEFAULT_PADDING;
        toolbar->isButtonPasteClicked = GuiButtonCustom((Rectangle){x, y, 24, 24}, "#18#", "PASTE", selectedItemCount <= 0, toolbar->ctx->disableGroundClick);

        rightStartx -= 24;
        toolbar->isButtonDeleteClicked = GuiButtonCustom((Rectangle){rightStartx, y, 24, 24}, "#143#", "DELETE", selectedItemCount <= 0, toolbar->ctx->disableGroundClick);

        GuiNewButton(&toolbar->newButtonProperty, toolbar->ctx);
    } else {
        toolbar->isButtonRestoreClicked = GuiButtonCustom((Rectangle){x, y, 100, 24}, "#77# Restore", "RESTORE SELECTED ITEM", selectedItemCount <= 0, toolbar->ctx->disableGroundClick);

        int prevBgColor = GuiGetStyle(DEFAULT, BACKGROUND_COLOR);
        int prevTextColor = GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL);
        int prevBorderColor = GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL);
        int prevFocusedBgColor = GuiGetStyle(DEFAULT, BASE_COLOR_FOCUSED);
        int prevFocusedTextColor = GuiGetStyle(DEFAULT, TEXT_COLOR_FOCUSED);
        int prevFocusedBorderColor = GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED);
        int prevPressedBgColor = GuiGetStyle(DEFAULT, BASE_COLOR_PRESSED);
        int prevPressedTextColor = GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED);
        int prevPressedBorderColor = GuiGetStyle(DEFAULT, BORDER_COLOR_PRESSED);

        // Normal
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt((Color){230, 0, 0, 255}));
        GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
        GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, ColorToInt((Color){180, 0, 0, 255}));

        // Focused
        GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt((Color){200, 0, 0, 255}));
        GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, ColorToInt(WHITE));
        GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, ColorToInt((Color){120, 0, 0, 255}));

        // Pressed
        GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt((Color){150, 0, 0, 255}));
        GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, ColorToInt(GRAY));
        GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, ColorToInt((Color){120, 0, 0, 255}));

        toolbar->isButtonPermanentDeleteClicked = GuiButtonCustom((Rectangle){rightStartx - 150, y, 150, 24}, "#143# Permanent Delete", NULL, selectedItemCount <= 0, toolbar->ctx->disableGroundClick);

        // Restore
        GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, prevBgColor);
        GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, prevTextColor);
        GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, prevBorderColor);
        GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, prevFocusedBgColor);
        GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, prevFocusedTextColor);
        GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, prevFocusedBorderColor);
        GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, prevPressedBgColor);
        GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, prevPressedTextColor);
        GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, prevPressedBorderColor);
    }

    GuiLine((Rectangle){toolbar->currentZeroPosition.x, toolbar->currentZeroPosition.y + 24, toolbar->currentZeroPosition.width, 10}, NULL);
}
