#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "file_manager.h"
#include "gui/component.h"
#include "gui/navbar.h"
#include "macro.h"
#include "raygui.h"

void createNavbar(Navbar *navbar) {
    navbar->textboxPatheditMode = false;
    strcpy(navbar->textboxPath, "");

    navbar->textboxSearcheditMode = false;
    strcpy(navbar->textboxSearch, "");

    navbar->currentZeroPosition = (Rectangle){0};
}

void updateNavbar(Navbar *navbar, Rectangle currentZeroPosition, FileManager *fileManager) {
    navbar->fileManager = fileManager;
    if (navbar->textboxPatheditMode == false) {
        strcpy(navbar->textboxPath, fileManager->currentPath);
    }

    navbar->currentZeroPosition = currentZeroPosition;
}

void drawNavbar(Navbar *navbar) {
    float x = navbar->currentZeroPosition.x;
    float y = navbar->currentZeroPosition.y;
    float totalWidth = navbar->currentZeroPosition.width;

    float buttonSize = 24;
    float spacing = DEFAULT_PADDING;
    float searchBoxWidth = 170;
    float searchBoxX = x + totalWidth - searchBoxWidth;

    float buttonsTotalWidth = buttonSize * 3 + spacing * 2;

    float pathBoxStartX = x + buttonsTotalWidth + spacing;
    float pathBoxMaxWidth = searchBoxX - pathBoxStartX - spacing;
    float pathBoxWidth = (pathBoxMaxWidth >= 170) ? pathBoxMaxWidth : 170;

    searchBoxX = searchBoxX < pathBoxStartX + pathBoxWidth + spacing ? pathBoxStartX + pathBoxWidth + spacing : searchBoxX;

    GuiButtonCustom((Rectangle){x, y, buttonSize, buttonSize}, "#56#", "UNDO", false);
    GuiButtonCustom((Rectangle){x + buttonSize + spacing, y, buttonSize, buttonSize}, "#57#", "REDO", false);
    if (GuiButtonCustom((Rectangle){x + (buttonSize + spacing) * 2, y, buttonSize, buttonSize}, "#117#", "BACK", false))
        goBack(navbar->fileManager);

    if (GuiTextBoxCustom((Rectangle){pathBoxStartX, y, pathBoxWidth, buttonSize}, "#1#", NULL, navbar->textboxPath, MAX_STRING_LENGTH, &navbar->textboxPatheditMode, false)) {
        printf("\n\nSearch value: %s\n", navbar->textboxPath);

        Tree currentRoot = getCurrentRoot(navbar->fileManager);
        if (currentRoot == NULL) {
            printf("Tidak ada root yang ditemukan\n");
            return;
        }
        printf("Root name: %s\n", currentRoot->item.name);

        int length = strlen(".dir") + strlen(navbar->textboxPath) + 2;
        char *fullPath = malloc(length);
        if (fullPath == NULL) {
            printf(stderr, "Gagal alokasi memori\n");
            exit(1);
        }

        snprintf(fullPath, length, "%s/%s", ".dir", navbar->textboxPath);

        printf("Full path: %s\n", fullPath);

        Item itemToSearch = createItem(_getNameFromPath(navbar->textboxPath), fullPath, 0, ITEM_FILE, 0, 0, 0);
        printf("Item name: %s\n", itemToSearch.name);

        Tree foundTree = searchTree(currentRoot, itemToSearch);
        if (foundTree == NULL) {
            printf("File tidak ditemukan\n");
            return;
        }
        printf("Found item: %s\n", foundTree->item.name);
        goTo(navbar->fileManager, foundTree);
    };

    if (GuiTextBoxCustom((Rectangle){searchBoxX, y, searchBoxWidth, buttonSize}, "#42#", "Search Item", navbar->textboxSearch, MAX_STRING_LENGTH, &navbar->textboxSearcheditMode, false)) {
        printf("%s\n", navbar->textboxSearch);
    };

    GuiLine((Rectangle){x, y + buttonSize, totalWidth, 10}, NULL);
}
