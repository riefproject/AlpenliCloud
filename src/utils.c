#include "utils.h"
#include "body.h"
#include "file_manager.h"
#include "navbar.h"
#include "raylib.h"
#include "toolbar.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gui/ctx.h"

#define CONTROL_KEY_PRESSED IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)
#define SHIFT_KEY_PRESSED IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)
#define ALT_KEY_PRESSED IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)

void inputString(char **s) {
    char *temp = malloc(1);
    char c;
    int i = 0;
    while ((c = getchar()) != '\n') {
        temp = realloc(temp, i + 1);
        temp[i] = c;

        if (temp == NULL) {
            printf("[LOG] Memory allocation failed\n");
            break;
        }

        i++;
    }
    temp[i] = '\0';
    *s = temp;
}

void trimTrailingSlash(char *path) {
    int len = strlen(path);
    while (len > 0 && path[len - 1] == '/') {
        path[--len] = '\0';
    }
}

void ShortcutKeys(Context *ctx) {


    // COPY (Ctrl + C)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_C)) {
        copyFile(ctx->fileManager);
    }

    // PASTE (Ctrl + V)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_V)) {
        pasteFile(ctx->fileManager);
    }

    // CUT (Ctrl + X)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_X)) {
        cutFile(ctx->fileManager);
    }

    // UNDO (Ctrl + Z)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_Z)) {
        if (ctx->fileManager != NULL) {
            undo(ctx->fileManager);
            printf("[LOG] Undo shortcut activated\n");
        }
    }

    // REDO (Ctrl + Y)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_Y)) {
        if (ctx->fileManager != NULL) {
            redo(ctx->fileManager);
            printf("[LOG] Redo shortcut activated\n");
        }
    }

    // DELETE (Delete key atau Ctrl + Delete)
    if (IsKeyPressed(KEY_DELETE) || ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_DELETE))) {
        ctx->toolbar->isButtonDeleteActive = true;
        printf("[LOG] Delete shortcut activated\n");
    }

    // RENAME (F2)
    if (IsKeyPressed(KEY_F2)) {
        if (ctx->fileManager != NULL && ctx->fileManager->selectedItem.head != NULL) {
            Item *selectedItem = (Item *)ctx->fileManager->selectedItem.head->data;
            if (selectedItem != NULL) {
                printf("[LOG] Rename shortcut activated for: %s\n", selectedItem->name);
                // renameFile(ctx->fileManager, selectedItem->path, "new_name"); // Implement proper rename UI
            }
        }
    }

    // REFRESH (F5 atau Ctrl + R)
    if (IsKeyPressed(KEY_F5) || ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_R))) {
        refreshFileManager(ctx->fileManager);
    }

    // GO BACK (Backspace atau Alt + Left Arrow)
    if (!ctx->toolbar->newButtonProperty.showModal &&
        !ctx->navbar->textboxPatheditMode &&
        !ctx->navbar->textboxSearcheditMode &&
        (IsKeyPressed(KEY_BACKSPACE) || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_LEFT)))) {
        if (ctx->fileManager != NULL &&
            ctx->fileManager->treeCursor != NULL &&
            ctx->fileManager->treeCursor->parent != NULL) {
            goBack(ctx->fileManager);
        }
    }

    // SELECT ALL (Ctrl + A)
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_A)) {
        if (ctx->fileManager != NULL && ctx->fileManager->treeCursor != NULL) {
            if (!ctx->body->selectedAll) {
                selectAll(ctx->fileManager);
                ctx->body->selectedAll = true;
            } else {
                clearSelectedFile(ctx->fileManager);
                ctx->body->selectedAll = false;
            }
            printf("[LOG] Select all shortcut activated\n");
        }
    }

    // NEW FOLDER (Ctrl + Shift + N)
    if ((CONTROL_KEY_PRESSED) && (SHIFT_KEY_PRESSED) && IsKeyPressed(KEY_N)) {
        ctx->toolbar->newButtonProperty.selectedType = ITEM_FOLDER;
        ctx->toolbar->newButtonProperty.showModal = true;
        if (!ctx->toolbar->newButtonProperty.showModal) {
            char *name = ctx->toolbar->newButtonProperty.inputBuffer;
            char *dirPath = TextFormat(".dir/%s", ctx->fileManager->currentPath);
            createFile(ctx->fileManager, ITEM_FOLDER, dirPath, name);
        }
    }

    // NEW FILE (Ctrl + N)
    if (!ctx->toolbar->newButtonProperty.showModal &&
        (CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_N) && !(SHIFT_KEY_PRESSED)) {
        ctx->navbar->textboxPatheditMode = false;
        ctx->navbar->textboxSearcheditMode = false;
        if (ctx->fileManager != NULL) {
            ctx->toolbar->newButtonProperty.selectedType = ITEM_FILE;
            ctx->toolbar->newButtonProperty.showModal = true;
            if (!ctx->toolbar->newButtonProperty.showModal) {
                char *name = ctx->toolbar->newButtonProperty.inputBuffer;
                char *dirPath = TextFormat(".dir/%s", ctx->fileManager->currentPath);
                createFile(ctx->fileManager, ITEM_FILE, dirPath, name);
            }
        }
    }

    // CTRL+F / FIND
    if (!ctx->toolbar->newButtonProperty.showModal &&
        (CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_F)) {
        ctx->navbar->textboxPatheditMode = false;
        ctx->toolbar->newButtonProperty.showModal = false;
        if (ctx->navbar->textboxSearcheditMode) {
            ctx->navbar->textboxSearcheditMode = false;
            printf("[LOG] Search mode deactivated\n");
        } else {
            ctx->navbar->textboxSearcheditMode = true;
            printf("[LOG] Search mode activated\n");
        }
    }

    // CTRL+L / ADDRESS/PATH
    if (!ctx->toolbar->newButtonProperty.showModal &&
        (CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_L)) {
        ctx->navbar->textboxSearcheditMode = false;
        ctx->toolbar->newButtonProperty.showModal = false;
        if (ctx->navbar->textboxPatheditMode) {
            ctx->navbar->textboxPatheditMode = false;
            printf("[LOG] Path edit mode deactivated\n");
        } else {
            ctx->navbar->textboxPatheditMode = true;
            printf("[LOG] Path edit mode activated\n");
        }
    }

    // ARROW KEYS NAVIGATION (only when not in edit mode)
    if (!ctx->navbar->textboxPatheditMode && !ctx->navbar->textboxSearcheditMode && !ctx->toolbar->newButtonProperty.showModal) {
        if (IsKeyPressed(KEY_UP)) {
            // Move selection up
            if (ctx->body->focusedIndex > 0) {
                ctx->body->focusedIndex--;

                // Clear all selections and select the focused item
                clearSelectedFile(ctx->fileManager);

                // Find the item at focusedIndex and select it
                Tree cursor = ctx->fileManager->treeCursor->first_son;
                int currentIndex = 0;
                while (cursor != NULL && currentIndex < ctx->body->focusedIndex) {
                    cursor = cursor->next_brother;
                    currentIndex++;
                }
                if (cursor != NULL) {
                    cursor->item.selected = true;
                    selectFile(ctx->fileManager, &cursor->item);
                }

                printf("[LOG] Arrow up navigation - index: %d\n", ctx->body->focusedIndex);
            }
        }

        if (IsKeyPressed(KEY_DOWN)) {
            // Count total items
            Tree cursor = ctx->fileManager->treeCursor->first_son;
            int totalItems = 0;
            while (cursor != NULL) {
                totalItems++;
                cursor = cursor->next_brother;
            }

            if (ctx->body->focusedIndex < totalItems - 1) {
                ctx->body->focusedIndex++;

                clearSelectedFile(ctx->fileManager);

                cursor = ctx->fileManager->treeCursor->first_son;
                int currentIndex = 0;
                while (cursor != NULL && currentIndex < ctx->body->focusedIndex) {
                    cursor = cursor->next_brother;
                    currentIndex++;
                }
                if (cursor != NULL) {
                    cursor->item.selected = true;
                    selectFile(ctx->fileManager, &cursor->item);
                }

                printf("[LOG] Arrow down navigation - index: %d\n", ctx->body->focusedIndex);
            }
        }
    }

    // ENTER KEY (open file/folder)
    if (!ctx->navbar->textboxPatheditMode && !ctx->navbar->textboxSearcheditMode && !ctx->toolbar->newButtonProperty.showModal) {
        if (IsKeyPressed(KEY_ENTER)) {
            if (ctx->body->focusedIndex >= 0) {
                Tree cursor = ctx->fileManager->treeCursor->first_son;
                int currentIndex = 0;

                while (cursor != NULL && currentIndex < ctx->body->focusedIndex) {
                    cursor = cursor->next_brother;
                    currentIndex++;
                }

                if (cursor != NULL) {
                    Item item = cursor->item;

                    if (item.type == ITEM_FOLDER) {
                        goTo(ctx->fileManager, cursor);
                        ctx->body->focusedIndex = 0;
                        printf("[LOG] Keyboard action - enter folder: %s\n", item.name);
                    } else if (item.type == ITEM_FILE) {
                        windowsOpenWith(item.path);
                        printf("[LOG] Keyboard action - open file: %s\n", item.name);
                    }
                }
            }
        }
    }
}