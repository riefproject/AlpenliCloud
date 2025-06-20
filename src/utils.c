#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>

#include "file_manager.h"
#include "utils.h"
#include "body.h"
#include "ctx.h"

#include "raylib.h"

#include "navbar.h"
#include "toolbar.h"

#define CONTROL_KEY_PRESSED IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)
#define SHIFT_KEY_PRESSED IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)
#define ALT_KEY_PRESSED IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)

/*
================================================================================
    PRIVATE HELPER PROTOTYPES
================================================================================
*/

// Shortcuts
static void _handleClipboardOperations(Context* ctx);       // Handle CTRL+C, CTRL+V, CTRL+X shortcuts
static void _handleUndoRedoOperations(Context* ctx);        // Handle CTRL+Z, CTRL+Y shortcuts
static void _handleDeleteOperation(Context* ctx);           // Handle DELETE key or CTRL+DELETE shortcut
static void _handleRenameOperation(Context* ctx);           // Handle F2 key for renaming
static void _handleRefreshOperation(Context* ctx);          // Handle F5 key or CTRL+R for refreshing
static void _handleBackNavigation(Context* ctx);            // Handle BACKSPACE or ALT+LEFT for navigating back
static void _handleSelectAllOperation(Context* ctx);        // Handle CTRL+A for selecting all items
static void _handleNewItemOperations(Context* ctx);         // Handle CTRL+N for new file, CTRL+SHIFT+N for new folder
static void _handleSearchAndPathOperations(Context* ctx);   // Handle CTRL+F for search and CTRL+L for path edit mode
static void _handleArrowNavigation(Context* ctx);           // Handle arrow keys for navigating through items
static void _handleEnterKey(Context* ctx);                  // Handle Enter key for folder/file opening

// Utilities
static int _getTotalItems(Context* ctx);                    // Count total items in current directory
static void _selectItemAtIndex(Context* ctx, int index);    // Select item at a specific index

/*
================================================================================
--------------------------------------------------------------------------------
                                IMPLEMENTATION
--------------------------------------------------------------------------------
================================================================================
*/

void inputString(char** s) {
    char* temp = malloc(1);
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

void trimTrailingSlash(char* path) {
    int len = strlen(path);
    while (len > 0 && path[len - 1] == '/') {
        path[--len] = '\0';
    }
}

void toLowerStr(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = (char)tolower((unsigned char)str[i]);
    }
}

void ShortcutKeys(Context* ctx) {
    _handleClipboardOperations(ctx);
    _handleUndoRedoOperations(ctx);
    _handleDeleteOperation(ctx);
    _handleRenameOperation(ctx);
    _handleRefreshOperation(ctx);
    _handleBackNavigation(ctx);
    _handleSelectAllOperation(ctx);
    _handleNewItemOperations(ctx);
    _handleSearchAndPathOperations(ctx);
    _handleArrowNavigation(ctx);
    _handleEnterKey(ctx);
}

/*
================================================================================
    PRIVATE HELPER FUNCTIONS - SHORTCUTS OPERATIONS
================================================================================
*/

static void _handleClipboardOperations(Context* ctx) {
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_C)) {
        copyFile(ctx->fileManager);
    }

    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_V)) {
        pasteFile(ctx->fileManager, true);
    }

    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_X)) {
        cutFile(ctx->fileManager);
    }
}

static void _handleUndoRedoOperations(Context* ctx) {
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_Z)) {
        if (ctx->fileManager != NULL) {
            undo(ctx->fileManager);
            printf("[LOG] Undo shortcut activated\n");
        }
    }

    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_Y)) {
        if (ctx->fileManager != NULL) {
            redo(ctx->fileManager);
            printf("[LOG] Redo shortcut activated\n");
        }
    }
}

static void _handleDeleteOperation(Context* ctx) {
    if (IsKeyPressed(KEY_DELETE) || ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_DELETE))) {
        ctx->toolbar->isButtonDeleteClicked = true;
        printf("[LOG] Delete shortcut activated\n");
    }
}

static void _handleRenameOperation(Context* ctx) {
    if (IsKeyPressed(KEY_F2)) {
        if (ctx->fileManager != NULL && ctx->fileManager->selectedItem.head != NULL) {
            Item* selectedItem = (Item*)ctx->fileManager->selectedItem.head->data;
            if (selectedItem != NULL) {
                printf("[LOG] Rename shortcut activated for: %s\n", selectedItem->name);
                // renameFile(ctx->fileManager, selectedItem->path, "new_name"); // Implement proper rename UI
            }
        }
    }
}

static void _handleRefreshOperation(Context* ctx) {
    if (IsKeyPressed(KEY_F5) || ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_R))) {
        refreshFileManager(ctx->fileManager);
    }
}

static void _handleBackNavigation(Context* ctx) {
    if (!ctx->disableGroundClick &&
        !ctx->navbar->textboxPatheditMode &&
        !ctx->navbar->textboxSearcheditMode &&
        (IsKeyPressed(KEY_BACKSPACE) || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_LEFT)))) {
        if (ctx->fileManager != NULL &&
            ctx->fileManager->treeCursor != NULL &&
            ctx->fileManager->treeCursor->parent != NULL) {
            goBack(ctx->fileManager);
        }
    }
}

static void _handleSelectAllOperation(Context* ctx) {
    if ((CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_A)) {
        if (ctx->fileManager != NULL && ctx->fileManager->treeCursor != NULL) {
            if (!ctx->body->selectedAll) {
                selectAll(ctx->fileManager);
                ctx->body->selectedAll = true;
            }
            else {
                clearSelectedFile(ctx->fileManager);
                ctx->body->selectedAll = false;
            }
            printf("[LOG] Select all shortcut activated\n");
        }
    }
}

static void _handleNewItemOperations(Context* ctx) {
    // NEW FOLDER (Ctrl + Shift + N)
    if ((CONTROL_KEY_PRESSED) && (SHIFT_KEY_PRESSED) && IsKeyPressed(KEY_N)) {
        ctx->toolbar->selectedType = ITEM_FOLDER;
        ctx->disableGroundClick = true;
        if (!ctx->disableGroundClick) {
            char* name = ctx->toolbar->inputCreateItemBuffer;
            char* dirPath = TextFormat(".dir/%s", ctx->fileManager->currentPath);
            createFile(ctx->fileManager, ITEM_FOLDER, dirPath, name, true);
        }
    }

    // NEW FILE (Ctrl + N)
    if (!ctx->disableGroundClick &&
        (CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_N) && !(SHIFT_KEY_PRESSED)) {
        ctx->navbar->textboxPatheditMode = false;
        ctx->navbar->textboxSearcheditMode = false;
        if (ctx->fileManager != NULL) {
            ctx->toolbar->selectedType = ITEM_FILE;
            ctx->disableGroundClick = true;
            if (!ctx->disableGroundClick) {
                char* name = ctx->toolbar->inputCreateItemBuffer;
                char* dirPath = TextFormat(".dir/%s", ctx->fileManager->currentPath);
                createFile(ctx->fileManager, ITEM_FILE, dirPath, name, true);
            }
        }
    }
}

static void _handleSearchAndPathOperations(Context* ctx) {
    // CTRL+F / FIND
    if (!ctx->disableGroundClick && (CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_F)) {
        ctx->navbar->textboxPatheditMode = false;
        ctx->disableGroundClick = false;
        if (ctx->navbar->textboxSearcheditMode) {
            ctx->navbar->textboxSearcheditMode = false;
            printf("[LOG] Search mode deactivated\n");
        }
        else {
            ctx->navbar->textboxSearcheditMode = true;
            printf("[LOG] Search mode activated\n");
        }
    }

    // CTRL+L / ADDRESS/PATH
    if (!ctx->disableGroundClick && (CONTROL_KEY_PRESSED) && IsKeyPressed(KEY_L)) {
        ctx->navbar->textboxSearcheditMode = false;
        ctx->disableGroundClick = false;
        if (ctx->navbar->textboxPatheditMode) {
            ctx->navbar->textboxPatheditMode = false;
            printf("[LOG] Path edit mode deactivated\n");
        }
        else {
            ctx->navbar->textboxPatheditMode = true;
            printf("[LOG] Path edit mode activated\n");
        }
    }
}

static void _handleArrowNavigation(Context* ctx) {
    if (!ctx->navbar->textboxPatheditMode && !ctx->navbar->textboxSearcheditMode && !ctx->disableGroundClick) {
        if (IsKeyPressed(KEY_UP)) {
            if (ctx->body->focusedIndex > 0) {
                ctx->body->focusedIndex--;
                _selectItemAtIndex(ctx, ctx->body->focusedIndex);
                printf("[LOG] Arrow up navigation - index: %d\n", ctx->body->focusedIndex);
            }
        }

        if (IsKeyPressed(KEY_DOWN)) {
            int totalItems = _getTotalItems(ctx);
            if (ctx->body->focusedIndex < totalItems - 1) {
                ctx->body->focusedIndex++;
                _selectItemAtIndex(ctx, ctx->body->focusedIndex);
                printf("[LOG] Arrow down navigation - index: %d\n", ctx->body->focusedIndex);
            }
        }
    }
}

static void _handleEnterKey(Context* ctx) {
    if (!ctx->navbar->textboxPatheditMode && !ctx->navbar->textboxSearcheditMode && !ctx->disableGroundClick) {
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
                    }
                    else if (item.type == ITEM_FILE) {
                        windowsOpenWith(item.path);
                        printf("[LOG] Keyboard action - open file: %s\n", item.name);
                    }
                }
            }
        }
    }
}

static void _selectItemAtIndex(Context* ctx, int index) {
    clearSelectedFile(ctx->fileManager);

    Tree cursor = ctx->fileManager->treeCursor->first_son;
    int currentIndex = 0;
    while (cursor != NULL && currentIndex < index) {
        cursor = cursor->next_brother;
        currentIndex++;
    }
    if (cursor != NULL) {
        cursor->item.selected = true;
        selectFile(ctx->fileManager, &cursor->item);
    }
}

static int _getTotalItems(Context* ctx) {
    Tree cursor = ctx->fileManager->treeCursor->first_son;
    int totalItems = 0;
    while (cursor != NULL) {
        totalItems++;
        cursor = cursor->next_brother;
    }
    return totalItems;
}
