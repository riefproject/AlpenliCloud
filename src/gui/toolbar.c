#include <stdbool.h>
#include <string.h>

#include "ctx.h"
#include "gui/component.h"
#include "gui/sidebar.h"
#include "gui/toolbar.h"
#include "win_utils.h" // Include Windows utils

#include "macro.h"
#include "raygui.h"

void createToolbar(Toolbar *toolbar, Context *ctx) {
    toolbar->ctx = ctx;

    toolbar->currentZeroPosition = (Rectangle){0};

    toolbar->isButtonCopyClicked = false;
    toolbar->isButtonCutClicked = false;
    toolbar->isButtonDeleteClicked = false;
    toolbar->isButtonPasteClicked = false;
    toolbar->isbuttonRenameClicked = false;
    toolbar->isButtonRestoreClicked = false;
    toolbar->isButtonPermanentDeleteClicked = false;
    toolbar->isButtonImportClicked = false;
    toolbar->isButtonCreateItemClicked = false;

    toolbar->showCreateModal = false;
    toolbar->createItemModalResult = false;
    toolbar->dropdownActive = false;
    toolbar->inputCreateItemEditMode = false;
    toolbar->inputCreateItemBuffer[0] = '\0';

    toolbar->selectedType = ITEM_FOLDER; // Default type for new items

    toolbar->showRenameModal = false;
    toolbar->renameModalResult = false;
    toolbar->renameInputBuffer[0] = '\0';

    toolbar->showImportModal = false;
    toolbar->importModalResult = false;
    toolbar->importPath[0] = '\0';
    // Menambahkan fitur import baru
    toolbar->importClicked = false;
    toolbar->showImportModal = false;
    toolbar->importModalResult = false;
    memset(toolbar->importPath, 0, sizeof(toolbar->importPath));
}

void updateToolbar(Toolbar *toolbar, Context *ctx) {
    toolbar->ctx = ctx;
    ctx->toolbar = toolbar;

    toolbar->currentZeroPosition = *ctx->currentZeroPosition;
    toolbar->currentZeroPosition.y += 24 + DEFAULT_PADDING;

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
        pasteFile(ctx->fileManager, true);
        toolbar->isButtonPasteClicked = false;
    }

    if (toolbar->isButtonRestoreClicked) {
        toolbar->isButtonRestoreClicked = false;
        recoverFile(ctx->fileManager);
    }

    if (toolbar->isButtonPermanentDeleteClicked) {
        toolbar->isButtonPermanentDeleteClicked = false;

        if (ctx->fileManager->isRootTrash) {
            deletePermanentFile(ctx->fileManager);
        } else {
            printf("[ERROR] Permanent delete can only be used in Trash.\n");
        }
    }

    if (toolbar->isButtonCreateItemClicked) {
        toolbar->isButtonCreateItemClicked = false;
        toolbar->dropdownActive = !toolbar->dropdownActive;
    }

    if (toolbar->createItemModalResult) {
        char *name = toolbar->inputCreateItemBuffer;
        char *dirPath = ctx->fileManager->treeCursor
            ? ctx->fileManager->treeCursor->item.path
            : TextFormat(".dir/%s", ctx->fileManager->currentPath);
        createFile(ctx->fileManager, toolbar->selectedType, dirPath, name, true);

        toolbar->createItemModalResult = false;
        toolbar->showCreateModal = false;
        toolbar->dropdownActive = false;
        toolbar->inputCreateItemEditMode = false;
        toolbar->inputCreateItemBuffer[0] = '\0';
        toolbar->ctx->disableGroundClick = false;
    }

    if (toolbar->isbuttonRenameClicked) {
        Item *selectedItem = ctx->fileManager->selectedItem.head ? (Item *)ctx->fileManager->selectedItem.head->data : NULL;

        if (selectedItem) {
            strncpy(toolbar->renameInputBuffer, selectedItem->name, MAX_STRING_LENGTH - 1);
            toolbar->renameInputBuffer[MAX_STRING_LENGTH - 1] = '\0';
            toolbar->selectedType = selectedItem->type;
            toolbar->showRenameModal = true;
            toolbar->renameModalResult = false;
        } else {
            toolbar->renameInputBuffer[0] = '\0';
        }

        toolbar->isbuttonRenameClicked = false;
    }

    if (toolbar->renameModalResult) {
        char *newName = toolbar->renameInputBuffer;
        Item *item = ctx->fileManager->selectedItem.head ? (Item *)ctx->fileManager->selectedItem.head->data : NULL;

        if (item && strlen(newName) > 0) {
            char *filePath = TextFormat(".dir/%s/%s", ctx->fileManager->currentPath, item->name);
            printf("Renaming %s → %s\n", filePath, newName);

            renameFile(ctx->fileManager, filePath, newName, true);
        } else {
            printf("[ERROR] No item selected or empty new name.\n");
        }

        toolbar->renameInputBuffer[0] = '\0';
        toolbar->renameModalResult = false;
    }

    // Menambahkan handling untuk import
    if (toolbar->isButtonImportClicked) {
        toolbar->isButtonImportClicked = false;
        toolbar->showImportModal = true;
        toolbar->importPath[0] = '\0'; // Clear path
        printf("[LOG] Import modal activated\n");
    }

    // Handle import modal result
    if (toolbar->importModalResult) {
        toolbar->importModalResult = false;

        if (strlen(toolbar->importPath) > 0) {
            importFile(ctx->fileManager, toolbar->importPath, true);
            printf("[LOG] Import file operation would execute: %s\n", toolbar->importPath);
        }

        // Clear after use
        toolbar->importPath[0] = '\0';
    }
}

void drawToolbar(Toolbar *toolbar) {
    float x = toolbar->currentZeroPosition.x;
    float y = toolbar->currentZeroPosition.y;
    float width = toolbar->currentZeroPosition.width;

    int rightStartx = x + width;
    int selectedItemCount = get_length(toolbar->ctx->fileManager->selectedItem);

    GuiLine((Rectangle){toolbar->currentZeroPosition.x, toolbar->currentZeroPosition.y + 24, toolbar->currentZeroPosition.width, 10}, NULL);

    // printf("[LOG] Selected Item Count: %d, boolean: %d\n", selectedItemCount, selectedItemCount <= 0);

    if (!toolbar->ctx->fileManager->isRootTrash) {
        toolbar->isButtonCreateItemClicked = GuiButtonCustom((Rectangle){x, y, 100, 24}, "#65# New", "Create Item", false, toolbar->ctx->disableGroundClick);
        DrawNewButtonDropdown(toolbar->ctx, (Rectangle){x, y, 100, 24});

        x += 100 + DEFAULT_PADDING;
        toolbar->isbuttonRenameClicked = GuiButtonCustom((Rectangle){x, y, 24, 24}, "#22#", "RENAME", (selectedItemCount <= 0 || selectedItemCount > 1), toolbar->ctx->disableGroundClick);

        x += 24 + DEFAULT_PADDING;
        toolbar->isButtonCutClicked = GuiButtonCustom((Rectangle){x, y, 24, 24}, "#17#", "CUT", selectedItemCount <= 0, toolbar->ctx->disableGroundClick);

        x += 24 + DEFAULT_PADDING;
        toolbar->isButtonCopyClicked = GuiButtonCustom((Rectangle){x, y, 24, 24}, "#16#", "COPY", selectedItemCount <= 0, toolbar->ctx->disableGroundClick);

        x += 24 + DEFAULT_PADDING;
        toolbar->isButtonPasteClicked = GuiButtonCustom((Rectangle){x, y, 24, 24}, "#18#", "PASTE", !(toolbar->ctx->fileManager->clipboard.front), toolbar->ctx->disableGroundClick);

        // Menambahkan import button
        x += 24 + DEFAULT_PADDING;
        toolbar->isButtonImportClicked = GuiButtonCustom((Rectangle){x, y, 50, 24}, "#132#", "IMPORT", false, toolbar->ctx->disableGroundClick);

        rightStartx -= 24;

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

        toolbar->isButtonDeleteClicked = GuiButtonCustom((Rectangle){rightStartx - 50, y, 75, 24}, "#143# Delete", "DELETE", selectedItemCount <= 0, toolbar->ctx->disableGroundClick);

        // Restore
        GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));
        GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));
        GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
        GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, GuiGetStyle(DEFAULT, BASE_COLOR_FOCUSED));
        GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, GuiGetStyle(DEFAULT, TEXT_COLOR_FOCUSED));
        GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED));
        GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, GuiGetStyle(DEFAULT, BASE_COLOR_PRESSED));
        GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED));
        GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, GuiGetStyle(DEFAULT, BORDER_COLOR_PRESSED));
    } else {
        toolbar->isButtonRestoreClicked = GuiButtonCustom((Rectangle){x, y, 100, 24}, "#77# Restore", "RESTORE SELECTED ITEM", selectedItemCount <= 0, toolbar->ctx->disableGroundClick);
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
        GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL));
        GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL));
        GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL));
        GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, GuiGetStyle(DEFAULT, BASE_COLOR_FOCUSED));
        GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, GuiGetStyle(DEFAULT, TEXT_COLOR_FOCUSED));
        GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED));
        GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, GuiGetStyle(DEFAULT, BASE_COLOR_PRESSED));
        GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED));
        GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, GuiGetStyle(DEFAULT, BORDER_COLOR_PRESSED));
    }
}

void DrawNewButtonDropdown(Context *ctx, Rectangle btnRect) {
    Toolbar *toolbar = ctx->toolbar;
    if (!toolbar->dropdownActive)
        return;

    float btnWidth = btnRect.width + 50;
    float btnHeight = btnRect.height;
    float totalHeight = (btnHeight * 2) + (DEFAULT_PADDING * 3);

    Rectangle dropdownBg = {
        btnRect.x,
        btnRect.y + btnHeight + 2,
        btnWidth,
        totalHeight};

    DrawRectangleRec(dropdownBg, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    DrawRectangleLinesEx(dropdownBg, 1, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

    Rectangle fileBtn = {
        dropdownBg.x + DEFAULT_PADDING,
        dropdownBg.y + DEFAULT_PADDING,
        btnWidth - 2 * DEFAULT_PADDING,
        btnHeight};

    if (GuiButton(fileBtn, "#8# File")) {
        toolbar->selectedType = ITEM_FILE;
        toolbar->showCreateModal = true;
        toolbar->dropdownActive = false;
        ctx->disableGroundClick = true;
        strcpy(toolbar->inputCreateItemBuffer, "");
        toolbar->inputCreateItemEditMode = true;
    }

    Rectangle folderBtn = {
        fileBtn.x,
        fileBtn.y + btnHeight + DEFAULT_PADDING,
        fileBtn.width,
        btnHeight};

    if (GuiButton(folderBtn, "#204# Folder")) {
        toolbar->selectedType = ITEM_FOLDER;
        toolbar->showCreateModal = true;
        toolbar->dropdownActive = false;
        ctx->disableGroundClick = true;
        strcpy(toolbar->inputCreateItemBuffer, "");
        toolbar->inputCreateItemEditMode = true;
    }

    // Tutup jika klik di luar dropdown dan tombol
    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        !CheckCollisionPointRec(mouse, dropdownBg) &&
        !CheckCollisionPointRec(mouse, btnRect)) {
        toolbar->dropdownActive = false;
        ctx->disableGroundClick = false;
    }
}

void DrawNewItemModal(Context *ctx) {
    Toolbar *toolbar = ctx->toolbar;
    if (!toolbar->showCreateModal)
        return;

    ctx->disableGroundClick = true;

    // Ukuran dan posisi modal
    Rectangle modalRect = {300, 200, 300, 150};
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    modalRect.x = (screenW - modalRect.width) / 2;
    modalRect.y = (screenH - modalRect.height) / 2;

    bool created = false;

    // Latar belakang gelap
    DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.3f));

    // Judul
    const char *typeStr = (toolbar->selectedType == ITEM_FILE) ? "File" : "Folder";
    char title[64];
    snprintf(title, sizeof(title), "Create %s", typeStr);

    bool quit = GuiWindowBox(modalRect, title);

    // Input box
    Rectangle inputBox = {
        modalRect.x + 20,
        modalRect.y + 45,
        modalRect.width - 40,
        30};

    created = GuiTextBoxCustom(
        inputBox,
        NULL,
        "Enter name...",
        toolbar->inputCreateItemBuffer,
        MAX_STRING_LENGTH,
        &toolbar->inputCreateItemEditMode,
        false,
        false);

    // Tombol create dan cancel (hardcoded)
    Rectangle createBtn = {
        modalRect.x + 20,
        modalRect.y + modalRect.height - 45,
        120,
        30};

    Rectangle cancelBtn = {
        modalRect.x + modalRect.width - 140,
        createBtn.y,
        120,
        30};

    if (GuiButton(createBtn, "Create") || created) {
        toolbar->createItemModalResult = true;
        toolbar->showCreateModal = false;
        toolbar->inputCreateItemEditMode = false;
        ctx->disableGroundClick = false;
    }

    if (GuiButton(cancelBtn, "Cancel") || quit) {
        toolbar->createItemModalResult = false;
        toolbar->showCreateModal = false;
        toolbar->inputCreateItemEditMode = false;
        toolbar->inputCreateItemBuffer[0] = '\0';
        ctx->disableGroundClick = false;
    }
}

void DrawRenameItemModal(Context *ctx) {
    Toolbar *toolbar = ctx->toolbar;
    if (!toolbar->showRenameModal)
        return;

    ctx->disableGroundClick = true;

    Rectangle modalRect = {300, 200, 300, 150};
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.3f));

    ItemType type = toolbar->selectedType;
    const char *typeStr = (type == ITEM_FILE) ? "File" : "Folder";
    char title[64];
    snprintf(title, sizeof(title), "Rename %s", typeStr);

    bool quit = GuiWindowBox(modalRect, title);

    Rectangle inputBox = {
        modalRect.x + 20,
        modalRect.y + 45,
        modalRect.width - 40,
        30};

    bool created = false;

    created = GuiTextBoxCustom(inputBox, NULL, "New name...", toolbar->renameInputBuffer,
                               MAX_STRING_LENGTH, &toolbar->inputCreateItemEditMode, false, false);

    Rectangle createBtn = {
        modalRect.x + 20,
        modalRect.y + modalRect.height - 45,
        (modalRect.width - 50) / 2,
        30};
    Rectangle cancelBtn = {
        createBtn.x + createBtn.width + 10,
        createBtn.y,
        createBtn.width,
        30};

    if (GuiButton(createBtn, "Rename") || (created && strlen(toolbar->renameInputBuffer) > 0)) {
        toolbar->showRenameModal = false;
        toolbar->renameModalResult = true;
        ctx->disableGroundClick = false;
    }

    if (GuiButton(cancelBtn, "Cancel") || quit) {
        toolbar->showRenameModal = false;
        toolbar->renameModalResult = false;
        toolbar->renameInputBuffer[0] = '\0';
        ctx->disableGroundClick = false;
    }
}

void DrawImportModal(Context *ctx) {
    bool *showImportModal = &ctx->toolbar->showImportModal;
    char *importPath = ctx->toolbar->importPath;
    int pathSize = sizeof(ctx->toolbar->importPath);
    bool *modalResult = &ctx->toolbar->importModalResult;

    if (!*showImportModal) {
        return;
    }

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int modalWidth = 600;
    int modalHeight = 300;

    Rectangle modalRect = {
        (screenWidth - modalWidth) / 2.0f,
        (screenHeight - modalHeight) / 2.0f,
        modalWidth,
        modalHeight};

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
        25};
    GuiLabel(titleRect, "Import files or folders from outside the workspace");

    // Instructions
    Rectangle instructionRect = {
        modalRect.x + 20,
        modalRect.y + 70,
        modalWidth - 40,
        20};
    GuiLabel(instructionRect, "Enter the full path or use Browse button to select:");

    // Examples
    Rectangle exampleRect = {
        modalRect.x + 20,
        modalRect.y + 95,
        modalWidth - 40,
        15};
    GuiLabel(exampleRect, "Examples: C:/Users/YourName/Documents/file.txt or D:/MyFolder");

    // Path input
    Rectangle pathInputRect = {
        modalRect.x + 20,
        modalRect.y + 120,
        modalWidth - 130,
        30};

    static bool pathEditMode = true;
    if (GuiTextBox(pathInputRect, importPath, pathSize, pathEditMode)) {
        pathEditMode = !pathEditMode;
    }

    // Browse File button - menggunakan Windows utils
    Rectangle browseFileRect = {
        modalRect.x + modalWidth - 100,
        modalRect.y + 120,
        80,
        14};

    if (GuiButton(browseFileRect, "#5# File")) {
        char selectedPath[512] = {0};
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
        14};

    if (GuiButton(browseFolderRect, "#1# Folder")) {
        char selectedPath[512] = {0};
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
        20};
    GuiLabel(quickAccessRect, "Quick Access:");

    float btnWidth = 80;
    float btnSpacing = 10;
    float startX = modalRect.x + 20;
    float btnY = modalRect.y + 180;

    // Desktop button
    Rectangle desktopRect = {startX, btnY, btnWidth, 20};
    if (GuiButton(desktopRect, "Desktop")) {
        char desktopPath[512];
        if (GetWindowsCommonPath(WIN_FOLDER_DESKTOP, desktopPath, sizeof(desktopPath))) {
            strncpy(importPath, desktopPath, pathSize - 1);
            importPath[pathSize - 1] = '\0';
            printf("[LOG] Desktop path set: %s\n", importPath);
        }
    }

    // Documents button
    Rectangle documentsRect = {startX + (btnWidth + btnSpacing), btnY, btnWidth, 20};
    if (GuiButton(documentsRect, "Documents")) {
        char documentsPath[512];
        if (GetWindowsCommonPath(WIN_FOLDER_DOCUMENTS, documentsPath, sizeof(documentsPath))) {
            strncpy(importPath, documentsPath, pathSize - 1);
            importPath[pathSize - 1] = '\0';
            printf("[LOG] Documents path set: %s\n", importPath);
        }
    }

    // Downloads button
    Rectangle downloadsRect = {startX + 2 * (btnWidth + btnSpacing), btnY, btnWidth, 20};
    if (GuiButton(downloadsRect, "Downloads")) {
        char downloadsPath[512];
        if (GetWindowsCommonPath(WIN_FOLDER_DOWNLOADS, downloadsPath, sizeof(downloadsPath))) {
            strncpy(importPath, downloadsPath, pathSize - 1);
            importPath[pathSize - 1] = '\0';
            printf("[LOG] Downloads path set: %s\n", importPath);
        }
    }

    // Pictures button
    Rectangle picturesRect = {startX + 3 * (btnWidth + btnSpacing), btnY, btnWidth, 20};
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
        20};

    static bool showValidation = false;
    static char validationMsg[256] = "";

    if (strlen(importPath) > 0) {
        if (!ValidateWindowsPath(importPath)) {
            strcpy(validationMsg, "#143# Invalid path or file not found!");
            showValidation = true;
        } else {
            strcpy(validationMsg, "#84# Path is valid and ready to import");
            showValidation = true;
        }
    } else {
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
        30};

    Rectangle cancelBtnRect = {
        modalRect.x + modalWidth - 90,
        modalRect.y + modalHeight - 40,
        70,
        30};

    // Import button
    bool canImport = strlen(importPath) > 0 && ValidateWindowsPath(importPath);

    if (canImport) {
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(GREEN));
        GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
    } else {
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
