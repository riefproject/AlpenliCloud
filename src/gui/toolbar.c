#include <stdbool.h>
#include <string.h>

#include "ctx.h"
#include "gui/component.h"
#include "gui/sidebar.h"
#include "gui/toolbar.h"
#include "win_utils.h"  // Include Windows utils

#include "macro.h"
#include "raygui.h"

void createToolbar(Toolbar* toolbar, Context* ctx) {
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
        .disabled = false };
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
        .disabled = false };
    toolbar->currentZeroPosition = (Rectangle){ 0 };
    toolbar->isButtonCopyActive = false;
    toolbar->isButtonCutActive = false;
    toolbar->isButtonDeleteActive = false;
    toolbar->isButtonPasteActive = false;
    toolbar->isbuttonRenameActive = false;
    toolbar->importClicked = false;
}

void updateToolbar(Toolbar* toolbar, Context* ctx) {
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
        char* name = toolbar->newButtonProperty.inputBuffer;
        char* dirPath = TextFormat(".dir/%s", ctx->fileManager->currentPath);
        createFile(ctx->fileManager, toolbar->newButtonProperty.selectedType, dirPath, name, true);
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
        deleteFile(ctx->fileManager, true);
        toolbar->isButtonDeleteActive = false;
    }
    if (toolbar->isButtonPasteActive) {
        pasteFile(ctx->fileManager);
        toolbar->isButtonPasteActive = false;
    }
    if (toolbar->isbuttonRenameActive) {
        toolbar->renameButtonProperty.showModal = true;
        toolbar->renameButtonProperty.inputEditMode = true;
        Item* selectedItem = ctx->fileManager->selectedItem.head ? (Item*)ctx->fileManager->selectedItem.head->data : NULL;
        if (selectedItem) {
            strncpy(toolbar->renameButtonProperty.inputBuffer, selectedItem->name, MAX_STRING_LENGTH - 1);
            toolbar->renameButtonProperty.inputBuffer[MAX_STRING_LENGTH - 1] = '\0';
            toolbar->renameButtonProperty.selectedType = selectedItem->type;
        }
        else {
            toolbar->renameButtonProperty.inputBuffer[0] = '\0';
        }
    }

    if (toolbar->renameButtonProperty.itemCreated) {
        char* newName = toolbar->renameButtonProperty.inputBuffer;
        char* name = toolbar->ctx->fileManager->selectedItem.head ? ((Item*)toolbar->ctx->fileManager->selectedItem.head->data)->name : NULL;
        if (name == NULL || strlen(name) == 0) {
            printf("[ERROR] No item selected for renaming.\n");
            return;
        }

        char* filePath = TextFormat(".dir/%s/%s", ctx->fileManager->currentPath, name);

        printf("filePath: %s\n", filePath);

        renameFile(ctx->fileManager, filePath, newName, true);
        toolbar->renameButtonProperty.showModal = false;
        toolbar->renameButtonProperty.inputEditMode = false;
        toolbar->renameButtonProperty.inputBuffer[0] = '\0';
        toolbar->renameButtonProperty.itemCreated = false;
        toolbar->isbuttonRenameActive = false;
    }
    if (toolbar->importClicked) {
        toolbar->importClicked = false;

        // Tampilkan modal untuk input path import
        toolbar->showImportModal = true;
        toolbar->importPath[0] = '\0'; // Clear path
        printf("[LOG] Import modal activated\n");
    }

    // Handle import modal result
    if (toolbar->importModalResult) {
        toolbar->importModalResult = false;

        if (strlen(toolbar->importPath) > 0) {
            importFile(ctx->fileManager, toolbar->importPath, true);
            printf("[LOG] Import file operation executed: %s\n", toolbar->importPath);
        }

        // Clear after use
        toolbar->importPath[0] = '\0';
    }
}

void drawToolbar(Toolbar* toolbar) {
    float x = toolbar->currentZeroPosition.x;
    float y = toolbar->currentZeroPosition.y;
    float width = toolbar->currentZeroPosition.width;

    int rightStartx = x + width;
    int selectedItemCount = get_length(toolbar->ctx->fileManager->selectedItem);

    // Left side buttons
    x += toolbar->newButtonProperty.btnRect.width + DEFAULT_PADDING;
    toolbar->isbuttonRenameActive = GuiButtonCustom((Rectangle) { x, y, 24, 24 }, "#22#", "RENAME", selectedItemCount <= 0 || selectedItemCount > 1, toolbar->ctx->disableGroundClick);

    x += 24 + DEFAULT_PADDING;
    toolbar->isButtonCutActive = GuiButtonCustom((Rectangle) { x, y, 24, 24 }, "#17#", "CUT", selectedItemCount <= 0, toolbar->ctx->disableGroundClick);

    x += 24 + DEFAULT_PADDING;
    toolbar->isButtonCopyActive = GuiButtonCustom((Rectangle) { x, y, 24, 24 }, "#16#", "COPY", selectedItemCount <= 0, toolbar->ctx->disableGroundClick);

    x += 24 + DEFAULT_PADDING;
    toolbar->isButtonPasteActive = GuiButtonCustom((Rectangle) { x, y, 24, 24 }, "#18#", "PASTE", is_queue_empty(toolbar->ctx->fileManager->temp), toolbar->ctx->disableGroundClick);

    // Import button
    x += 24 + DEFAULT_PADDING;
    if (GuiButtonCustom((Rectangle) { x, y, 50, 24 }, "#132#", "IMPORT", false, toolbar->ctx->disableGroundClick)) {
        toolbar->importClicked = true;
    }

    // Right side buttons
    rightStartx -= 24;
    toolbar->isButtonDeleteActive = GuiButtonCustom((Rectangle) { rightStartx, y, 24, 24 }, "#143#", "DELETE", selectedItemCount <= 0, toolbar->ctx->disableGroundClick);

    GuiNewButton(&toolbar->newButtonProperty, toolbar->ctx);

    // Draw import modal if active
    if (toolbar->showImportModal) {
        DrawImportModal(toolbar->ctx, &toolbar->showImportModal, toolbar->importPath, sizeof(toolbar->importPath), &toolbar->importModalResult);
    }

    GuiLine((Rectangle) { toolbar->currentZeroPosition.x, toolbar->currentZeroPosition.y + 24, toolbar->currentZeroPosition.width, 10 }, NULL);
}

void DrawImportModal(Context* ctx, bool* showImportModal, char* importPath, int pathSize, bool* modalResult) {
    if (!*showImportModal) return;

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int modalWidth = 600;
    int modalHeight = 300;

    Rectangle modalRect = {
        (screenWidth - modalWidth) / 2.0f,
        (screenHeight - modalHeight) / 2.0f,
        modalWidth,
        modalHeight
    };

    // Background overlay
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.4f));

    // Modal window
    bool shouldClose = GuiWindowBox(modalRect, "#132# Import File/Folder");

    if (shouldClose || IsKeyPressed(KEY_ESCAPE)) {
        *showImportModal = false;
        return;
    }

    // Title
    Rectangle titleRect = {
        modalRect.x + 20,
        modalRect.y + 40,
        modalWidth - 40,
        25
    };
    GuiLabel(titleRect, "Import files or folders from outside the workspace");

    // Instructions
    Rectangle instructionRect = {
        modalRect.x + 20,
        modalRect.y + 70,
        modalWidth - 40,
        20
    };
    GuiLabel(instructionRect, "Enter the full path or use Browse button to select:");

    // Examples
    Rectangle exampleRect = {
        modalRect.x + 20,
        modalRect.y + 95,
        modalWidth - 40,
        15
    };
    GuiLabel(exampleRect, "Examples: C:/Users/YourName/Documents/file.txt or D:/MyFolder");

    // Path input
    Rectangle pathInputRect = {
        modalRect.x + 20,
        modalRect.y + 120,
        modalWidth - 130,
        30
    };

    static bool pathEditMode = true;
    if (GuiTextBox(pathInputRect, importPath, pathSize, pathEditMode)) {
        pathEditMode = !pathEditMode;
    }

    // Browse File button - menggunakan Windows utils
    Rectangle browseFileRect = {
        modalRect.x + modalWidth - 100,
        modalRect.y + 120,
        80,
        14
    };

    if (GuiButton(browseFileRect, "#5# File")) {
        char selectedPath[512] = { 0 };
        if (OpenWindowsFileDialog(selectedPath, sizeof(selectedPath))) {
            strncpy(importPath, selectedPath, pathSize - 1);
            importPath[pathSize - 1] = '\0';
            printf("[LOG] File selected: %s\n", selectedPath);
        }
    }

    // Browse Folder button - menggunakan Windows utils
    Rectangle browseFolderRect = {
        modalRect.x + modalWidth - 100,
        modalRect.y + 136,
        80,
        14
    };

    if (GuiButton(browseFolderRect, "#1# Folder")) {
        char selectedPath[512] = { 0 };
        if (OpenWindowsFolderDialog(selectedPath, sizeof(selectedPath))) {
            strncpy(importPath, selectedPath, pathSize - 1);
            importPath[pathSize - 1] = '\0';
            printf("[LOG] Folder selected: %s\n", selectedPath);
        }
    }

    // Quick access buttons - menggunakan Windows utils
    Rectangle quickAccessRect = {
        modalRect.x + 20,
        modalRect.y + 160,
        modalWidth - 40,
        20
    };
    GuiLabel(quickAccessRect, "Quick Access:");

    float btnWidth = 80;
    float btnSpacing = 10;
    float startX = modalRect.x + 20;
    float btnY = modalRect.y + 180;

    // Desktop button
    Rectangle desktopRect = { startX, btnY, btnWidth, 20 };
    if (GuiButton(desktopRect, "Desktop")) {
        char desktopPath[512];
        if (GetWindowsCommonPath(WIN_FOLDER_DESKTOP, desktopPath, sizeof(desktopPath))) {
            strncpy(importPath, desktopPath, pathSize - 1);
            importPath[pathSize - 1] = '\0';
            printf("[LOG] Desktop path set: %s\n", importPath);
        }
    }

    // Documents button
    Rectangle documentsRect = { startX + (btnWidth + btnSpacing), btnY, btnWidth, 20 };
    if (GuiButton(documentsRect, "Documents")) {
        char documentsPath[512];
        if (GetWindowsCommonPath(WIN_FOLDER_DOCUMENTS, documentsPath, sizeof(documentsPath))) {
            strncpy(importPath, documentsPath, pathSize - 1);
            importPath[pathSize - 1] = '\0';
            printf("[LOG] Documents path set: %s\n", importPath);
        }
    }

    // Downloads button
    Rectangle downloadsRect = { startX + 2 * (btnWidth + btnSpacing), btnY, btnWidth, 20 };
    if (GuiButton(downloadsRect, "Downloads")) {
        char downloadsPath[512];
        if (GetWindowsCommonPath(WIN_FOLDER_DOWNLOADS, downloadsPath, sizeof(downloadsPath))) {
            strncpy(importPath, downloadsPath, pathSize - 1);
            importPath[pathSize - 1] = '\0';
            printf("[LOG] Downloads path set: %s\n", importPath);
        }
    }

    // Pictures button
    Rectangle picturesRect = { startX + 3 * (btnWidth + btnSpacing), btnY, btnWidth, 20 };
    if (GuiButton(picturesRect, "Pictures")) {
        char picturesPath[512];
        if (GetWindowsCommonPath(WIN_FOLDER_PICTURES, picturesPath, sizeof(picturesPath))) {
            strncpy(importPath, picturesPath, pathSize - 1);
            importPath[pathSize - 1] = '\0';
            printf("[LOG] Pictures path set: %s\n", importPath);
        }
    }

    // Validation message - menggunakan Windows utils
    Rectangle validationRect = {
        modalRect.x + 20,
        modalRect.y + 210,
        modalWidth - 40,
        20
    };

    static bool showValidation = false;
    static char validationMsg[256] = "";

    if (strlen(importPath) > 0) {
        if (!ValidateWindowsPath(importPath)) {
            strcpy(validationMsg, "#143# Invalid path or file not found!");
            showValidation = true;
        }
        else {
            strcpy(validationMsg, "#84# Path is valid and ready to import");
            showValidation = true;
        }
    }
    else {
        showValidation = false;
    }

    if (showValidation) {
        Color msgColor = strstr(validationMsg, "Invalid") ? RED : GREEN;
        GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(msgColor));
        GuiLabel(validationRect, validationMsg);
        GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(DARKGRAY)); // Reset
    }

    // Buttons
    Rectangle importBtnRect = {
        modalRect.x + modalWidth - 180,
        modalRect.y + modalHeight - 40,
        80,
        30
    };

    Rectangle cancelBtnRect = {
        modalRect.x + modalWidth - 90,
        modalRect.y + modalHeight - 40,
        70,
        30
    };

    // Import button
    bool canImport = strlen(importPath) > 0 && ValidateWindowsPath(importPath);

    if (canImport) {
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(GREEN));
        GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
    }
    else {
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(GRAY));
        GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(LIGHTGRAY));
    }

    if (GuiButton(importBtnRect, "#84# Import") && canImport) {
        *modalResult = true;
        *showImportModal = false;
        printf("[LOG] Import confirmed for: %s\n", importPath);
    }

    // Reset button style
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(LIGHTGRAY));
    GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(DARKGRAY));

    // Handle Enter key
    if (IsKeyPressed(KEY_ENTER) && canImport) {
        *modalResult = true;
        *showImportModal = false;
        printf("[LOG] Import confirmed with Enter: %s\n", importPath);
    }

    // Cancel button
    if (GuiButton(cancelBtnRect, "#143# Cancel")) {
        *showImportModal = false;
        memset(importPath, 0, pathSize);
    }
}