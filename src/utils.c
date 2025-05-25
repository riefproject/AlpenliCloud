#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "raylib.h"
#include "toolbar.h"
#include "navbar.h"
#include "body.h"
#include "file_manager.h"

#define CONTROL_KEY_PRESSED IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)
#define SHIFT_KEY_PRESSED IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)
#define ALT_KEY_PRESSED IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)

void inputString(char** s) {
    char* temp = malloc(1);
    char c;
    int i = 0;
    while ((c = getchar()) != '\n') {
        temp = realloc(temp, i + 1);
        temp[i] = c;

        if (temp == NULL) {
            printf("Memory allocation failed\n");
            break;
        }

        i++;
    }
    temp[i] = '\0';
    *s = temp;
}

void ShortcutKeys(Toolbar* toolbar, Navbar* navbar, Body* body) {

    // COPY (Ctrl + C)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_C)) {
        copyFile(toolbar->fileManager);
    }

    // PASTE (Ctrl + V)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_V)) {
        pasteFile(toolbar->fileManager);
    }

    // CUT (Ctrl + X)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_X)) {
        cutFile(toolbar->fileManager);
    }

    // UNDO (Ctrl + Z)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_Z)) {
        if (toolbar->fileManager != NULL) {
            undo(toolbar->fileManager);
            printf("Undo shortcut activated\n");
        }
    }

    // REDO (Ctrl + Y)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_Y)) {
        if (toolbar->fileManager != NULL) {
            redo(toolbar->fileManager);
            printf("Redo shortcut activated\n");
        }
    }

    // DELETE (Delete key atau Ctrl + Delete)
    if (IsKeyPressed(KEY_DELETE) || ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_DELETE))) {
        toolbar->isButtonDeleteActive = true;
        printf("Delete shortcut activated\n");
    }

    // RENAME (F2)
    if (IsKeyPressed(KEY_F2)) {
        if (toolbar->fileManager != NULL && toolbar->fileManager->selectedItem.head != NULL) {
            Item* selectedItem = (Item*)toolbar->fileManager->selectedItem.head->data;
            if (selectedItem != NULL) {
                printf("Rename shortcut activated for: %s\n", selectedItem->name);
                // renameFile(toolbar->fileManager, selectedItem->path, "new_name"); // Implement proper rename UI
            }
        }
    }

    // REFRESH (F5 atau Ctrl + R)
    if (IsKeyPressed(KEY_F5) || ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_R))) {
        refreshFileManager(toolbar->fileManager);
    }

    // GO BACK (Backspace atau Alt + Left Arrow)
    if (!toolbar->newButtonProperty.showModal &&
        !navbar->textboxPatheditMode &&
        !navbar->textboxSearcheditMode &&
        (IsKeyPressed(KEY_BACKSPACE) || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_LEFT)))) {
        if (toolbar->fileManager != NULL &&
            toolbar->fileManager->treeCursor != NULL &&
            toolbar->fileManager->treeCursor->parent != NULL) {
            goBack(toolbar->fileManager);
        }
    }

    // SELECT ALL (Ctrl + A)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_A)) {
        if (toolbar->fileManager != NULL && toolbar->fileManager->treeCursor != NULL) {
            if (!body->selectedAll) {
                selectAll(toolbar->fileManager);
                body->selectedAll = true;
            }
            else {
                clearSelectedFile(toolbar->fileManager);
                body->selectedAll = false;
            }
            printf("Select all shortcut activated\n");
        }
    }

    // NEW FOLDER (Ctrl + Shift + N)
    if ((CONTROL_KEY_PRESSED) && (SHIFT_KEY_PRESSED) && IsKeyPressed(KEY_N)) {
        toolbar->newButtonProperty.selectedType = ITEM_FOLDER;
        toolbar->newButtonProperty.showModal = true;
        if (!toolbar->newButtonProperty.showModal) {
            char* name = toolbar->newButtonProperty.inputBuffer;
            char* dirPath = TextFormat(".dir/%s", toolbar->fileManager->currentPath);
            createFile(toolbar->fileManager, ITEM_FOLDER, dirPath, name);
        }
    }

    // NEW FILE (Ctrl + N)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_N) && !IsKeyDown(KEY_LEFT_SHIFT)) {
        if (toolbar->fileManager != NULL) {
            toolbar->newButtonProperty.selectedType = ITEM_FILE;
            toolbar->newButtonProperty.showModal = true;
            if (!toolbar->newButtonProperty.showModal) {
                char* name = toolbar->newButtonProperty.inputBuffer;
                char* dirPath = TextFormat(".dir/%s", toolbar->fileManager->currentPath);
                createFile(toolbar->fileManager, ITEM_FILE, dirPath, name);
            }
        }
    }

    // CTRL+F / FIND
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_F)) {
        if (navbar->textboxSearcheditMode) {
            navbar->textboxSearcheditMode = false;
            printf("Search mode deactivated\n");
        }
        else {
            navbar->textboxSearcheditMode = true;
            printf("Search mode activated\n");
        }
    }

    // CTRL+L / ADDRESS/PATH
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_L)) {
        if (navbar->textboxPatheditMode) {
            navbar->textboxPatheditMode = false;
            printf("Path edit mode deactivated\n");
        }
        else {
            navbar->textboxPatheditMode = true;
            printf("Path edit mode activated\n");
        }
    }
}