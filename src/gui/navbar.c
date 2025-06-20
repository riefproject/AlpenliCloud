#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raygui.h"

#include "file_manager.h"
#include "macro.h"

#include "ctx.h"
#include "gui/component.h"
#include "gui/navbar.h"

void trimTrailingSlash(char* path);

void createNavbar(Navbar* navbar, Context* ctx) {
    navbar->ctx = ctx;

    navbar->isUndoButtonClicked = false;
    navbar->isRedoButtonClicked = false;
    navbar->isGoBackButtonClicked = false;

    navbar->shouldGoToPath = false;
    navbar->textboxPatheditMode = false;
    strcpy(navbar->textboxPath, "");

    navbar->shouldSearch = false;
    navbar->textboxSearcheditMode = false;
    strcpy(navbar->textboxSearch, "");

    navbar->currentZeroPosition = (Rectangle){ 0 };
}

void updateNavbar(Navbar* navbar, Context* ctx) {
    navbar->ctx = ctx;
    ctx->navbar = navbar;

    navbar->currentZeroPosition = *ctx->currentZeroPosition;

    // Handle navigasi manual ke path
    if (navbar->shouldGoToPath) {
        navbar->shouldGoToPath = false;

        char trimmedPath[MAX_STRING_LENGTH];
        strncpy(trimmedPath, navbar->textboxPath, MAX_STRING_LENGTH);
        trimTrailingSlash(trimmedPath);

        Tree root = getCurrentRoot(*ctx->fileManager);
        if (!root) {
            printf("Root tidak ditemukan\n");
            return;
        }

        char* path = strdup(trimmedPath);
        char* subPath = strtok(path, "/");
        if (TextIsEqual(subPath, "trash")) {
            ctx->fileManager->isRootTrash = true;
            ctx->fileManager->currentPath = "trash";

            printf("[LOG] Opening Trash...\n");
            return;
        }
        else if (TextIsEqual(subPath, "root")) {
            ctx->fileManager->isRootTrash = false;
        }

        Item itemToSearch = createItem(
            getNameFromPath(trimmedPath),
            TextFormat("%s/%s", ".dir", trimmedPath),
            0, ITEM_FILE, 0, 0, 0);

        printf("[LOG] Searching for item: %s\n", itemToSearch.path);

        Tree result = searchTree(root, itemToSearch);
        if (result) {
            goTo(ctx->fileManager, result);
        }
        else {
            printf("File tidak ditemukan\n");
        }
    }

    if (navbar->shouldSearch) {
        navbar->shouldSearch = false;
        printf("Search string: %s\n", navbar->textboxSearch);

        if (TextIsEqual(navbar->textboxSearch, "")) {
            navbar->ctx->fileManager->isSearching = false;
            return;
        }

        ctx->fileManager->isSearching = true;

        if (ctx->fileManager->isRootTrash) {
            searchingLinkedListItem(ctx->fileManager, ctx->fileManager->trash.head, navbar->textboxSearch);
        }
        else {
            searchingTreeItem(ctx->fileManager, navbar->textboxSearch);
        }

        printSearchingList(ctx->fileManager);
    }

    // Handle undo button
    if (navbar->isUndoButtonClicked) {
        navbar->isUndoButtonClicked = false;
        undo(ctx->fileManager);
        // if (&navbar->ctx.fileManager) {
        //     // undo(&navbar->ctx.fileManager);
        // }
    }

    // Handle redo button
    if (navbar->isRedoButtonClicked) {
        navbar->isRedoButtonClicked = false;
        redo(ctx->fileManager);
        // if (&navbar->ctx.fileManager) {
        //     redo(&navbar->ctx.fileManager);
        // }
        // navbar->ctx.
    }

    // Handle go back button
    if (navbar->isGoBackButtonClicked) {
        navbar->isGoBackButtonClicked = false;
        goBack(ctx->fileManager);
    }

    // Sinkronkan path saat textbox tidak diedit
    if (!navbar->textboxPatheditMode) {
        strncpy(navbar->textboxPath, ctx->fileManager->currentPath, MAX_STRING_LENGTH);
    }
}

void drawNavbar(Navbar* navbar) {
    float x = navbar->currentZeroPosition.x;
    float y = navbar->currentZeroPosition.y;
    float totalWidth = navbar->currentZeroPosition.width;

    const float buttonSize = 24;
    const float spacing = DEFAULT_PADDING;
    const float searchBoxWidth = 170;

    float buttonsTotalWidth = buttonSize * 3 + spacing * 2;
    float pathBoxStartX = x + buttonsTotalWidth + spacing;
    float searchBoxX = x + totalWidth - searchBoxWidth;

    float pathBoxMaxWidth = searchBoxX - pathBoxStartX - spacing;
    float pathBoxWidth = (pathBoxMaxWidth >= 170) ? pathBoxMaxWidth : 170;

    if (searchBoxX < pathBoxStartX + pathBoxWidth + spacing) {
        searchBoxX = pathBoxStartX + pathBoxWidth + spacing;
    }

    navbar->isUndoButtonClicked = GuiButtonCustom((Rectangle) { x, y, buttonSize, buttonSize }, "#56#", "UNDO", false, navbar->ctx->disableGroundClick);
    navbar->isRedoButtonClicked = GuiButtonCustom((Rectangle) { x + buttonSize + spacing, y, buttonSize, buttonSize }, "#57#", "REDO", false, navbar->ctx->disableGroundClick);
    navbar->isGoBackButtonClicked = GuiButtonCustom((Rectangle) { x + (buttonSize + spacing) * 2, y, buttonSize, buttonSize }, "#117#", "BACK", false, navbar->ctx->disableGroundClick);

    navbar->shouldGoToPath = GuiTextBoxCustom(
        (Rectangle) {
        pathBoxStartX, y, pathBoxWidth, buttonSize
    },
        "#1#", NULL, navbar->textboxPath,
        MAX_STRING_LENGTH, & navbar->textboxPatheditMode, false, navbar->ctx->disableGroundClick);

    navbar->shouldSearch = GuiTextBoxCustom(
        (Rectangle) {
        searchBoxX, y, searchBoxWidth, buttonSize
    },
        "#42#", "Search Item", navbar->textboxSearch,
        MAX_STRING_LENGTH, & navbar->textboxSearcheditMode, false, navbar->ctx->disableGroundClick);

    GuiLine((Rectangle) { x, y + buttonSize, totalWidth, 10 }, NULL);
}
