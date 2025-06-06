#include "gui/body.h"
#include "ctx.h"
#include "file_manager.h"
#include "item.h"
#include "macro.h"

// #include <time.h>

#include "raygui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void createBody(Context *ctx, Body *b) {
    Body body = {0};
    body.ctx = ctx;
    body.panelRec = (Rectangle){0};
    body.panelContentRec = (Rectangle){0, 0, 170, 340};
    body.panelView = (Rectangle){0};
    body.panelScroll = (Vector2){0};

    body.focusedIndex = -1;
    body.showCheckbox = true;
    body.selectedAll = false;
    body.previousSelectedAll = false;

    *b = body;
}

void updateBody(Context *ctx, Body *body) {

    body->currentZeroPosition = *ctx->currentZeroPosition;

    body->panelRec = (Rectangle){
        body->currentZeroPosition.x + 170 + DEFAULT_PADDING,
        body->currentZeroPosition.y + DEFAULT_PADDING * 2 + 24 * 2,
        body->currentZeroPosition.width - 170 - DEFAULT_PADDING,
        body->currentZeroPosition.height - DEFAULT_PADDING * 2 - 24 * 2};

    if (ctx->fileManager->treeCursor) {
        Tree cursor = ctx->fileManager->treeCursor->first_son;
        int totalItems = 0;
        int selectedItems = 0;

        while (cursor != NULL) {
            totalItems++;
            if (cursor->item.selected) {
                selectedItems++;
            }
            cursor = cursor->next_brother;
        }

        if (totalItems == 0) {
            body->selectedAll = false;
        } else if (selectedItems == totalItems) {
            body->selectedAll = true;
        } else {
            body->selectedAll = false;
        }
    }
}

void drawBody(Context *ctx, Body *body) {
    Tree cursor = ctx->fileManager->treeCursor;

    sort_children(&cursor);

    cursor = cursor->first_son;

    float headerHeight = 30;
    float rowHeight = 24;

    float colWidths[5] = {300, 100, 100, 200};
    float checkboxWidth = body->showCheckbox ? 20 : 0;
    float totalContentWidth = checkboxWidth + colWidths[0] + colWidths[1] + colWidths[2] + colWidths[3];

    body->panelContentRec.width = totalContentWidth;

    if (ctx->disableGroundClick)
        GuiDisable();

    GuiScrollPanel(body->panelRec, NULL, body->panelContentRec, &body->panelScroll, &body->panelView);
    GuiEnable();

    BeginScissorMode(body->panelView.x, body->panelView.y, body->panelView.width, body->panelView.height);

    float startY = body->panelRec.y + headerHeight + body->panelScroll.y;
    float startX = body->panelRec.x + body->panelScroll.x;

    if (ctx->fileManager->isSearching || ctx->fileManager->isRootTrash) {
        // Jika tidak ada hasil pencarian
        if (ctx->fileManager->isSearching && ctx->fileManager->searchingList.head == NULL) {
            Rectangle noResultRec = {
                startX,
                startY,
                body->panelContentRec.width,
                body->panelContentRec.height - headerHeight};
            DrawRectangleRec(noResultRec, Fade(LIGHTGRAY, 0.5f));
            DrawText("Tidak ada hasil pencarian", startX + 10, startY + 10, 20, DARKGRAY);
        }

        // Jika ada trash kosong
        if (ctx->fileManager->trash.head == NULL && ctx->fileManager->isRootTrash) {
            Rectangle noTrashRec = {
                startX,
                startY,
                body->panelContentRec.width,
                body->panelContentRec.height - headerHeight};
            DrawRectangleRec(noTrashRec, Fade(LIGHTGRAY, 0.5f));
            DrawText("Trash kosong", startX + 10, startY + 10, 20, DARKGRAY);
        }

        int i = 0;
        Node *temp = ctx->fileManager->trash.head;
        if (ctx->fileManager->isSearching) {
            temp = ctx->fileManager->searchingList.head;
        }

        while (temp != NULL) {
            Tree treePtr = (Tree)temp->data;
            drawTableItem(ctx, body, treePtr, i, startX, body->panelRec.y + headerHeight + body->panelScroll.y, rowHeight, colWidths);
            temp = temp->next;
            i++;
        }
    } else {
        int i = 0;
        while (cursor != NULL) {
            drawTableItem(ctx, body, cursor, i, startX, body->panelRec.y + headerHeight + body->panelScroll.y, rowHeight, colWidths);

            if ((i + 1) * rowHeight + headerHeight > body->panelContentRec.height)
                body->panelContentRec.height = (i + 1) * rowHeight + headerHeight;
            i++;
            cursor = cursor->next_brother;
        }
    }

    float headerX = body->panelRec.x + body->panelScroll.x;
    drawTableHeader(ctx, body, headerX, body->panelRec.y, colWidths);

    EndScissorMode();
}

void drawTableItem(Context *ctx, Body *body, Tree subTree, int index, float startX, float startY, float rowHeight, float colWidths[5]) {
    Item item = subTree->item;
    float checkboxWidth = body->showCheckbox ? 28 : 0;
    float totalContentWidth = checkboxWidth + colWidths[0] + colWidths[1] + colWidths[2] + colWidths[3];

    float rowY = startY + index * rowHeight;
    float rowX = startX;

    Rectangle rowRec = {rowX + checkboxWidth, rowY, totalContentWidth - checkboxWidth, rowHeight};

    if (CheckCollisionPointRec(GetMousePosition(), rowRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !ctx->disableGroundClick) {
        body->focusedIndex = index;

        // Handle double tap untuk navigation/open
        if (GetGestureDetected() == GESTURE_DOUBLETAP && ctx->fileManager->isRootTrash == false) {
            if (item.type == ITEM_FOLDER) {
                ctx->fileManager->isSearching = false;
                // ctx->navbar
                goTo(ctx->fileManager, subTree);
            } else if (item.type == ITEM_FILE) {
                windowsOpenWith(item.path);
            }
        }
        // Handle single tap
        else if (GetGestureDetected() == GESTURE_TAP || GetGestureDetected() == GESTURE_NONE) {
            // Ctrl + Click untuk selection
            if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
                // Toggle selection status
                subTree->item.selected = !subTree->item.selected;

                if (subTree->item.selected) {
                    selectFile(ctx->fileManager, &subTree->item);
                } else {
                    deselectFile(ctx->fileManager, &subTree->item);
                }
            }
            // Single click tanpa Ctrl = clear selection dan focus item ini
            else {
                // Clear semua selection dulu
                clearSelectedFile(ctx->fileManager);

                // Select item yang diklik
                subTree->item.selected = true;
                selectFile(ctx->fileManager, &subTree->item);
            }
        }
    }

    Color bgColor;
    // Tolong cek warnanya dong. layar monitorku ga akurat warnanya
    if (subTree->item.selected) {
        // Item yang di-select: biru terang (seperti Windows Explorer)
        bgColor = Fade(BLUE, 0.3f);
    } else if (body->focusedIndex == index) {
        // Item yang di-focus tapi tidak selected: biru lebih gelap
        bgColor = Fade(BLUE, 0.15f);
    } else {
        // Alternating row colors untuk yang tidak selected/focused
        bgColor = (index % 2 == 0) ? WHITE : (Color){245, 245, 245, 255};
    }

    rowRec = (Rectangle){rowX, rowY, totalContentWidth, rowHeight};

    DrawRectangleRec(rowRec, bgColor);

    float colX = rowX;

    if (body->showCheckbox) {
        Rectangle checkBox = {
            colX + 7,
            rowY + (rowHeight - 14) / 2,
            14, 14};

        // PERBAIKAN: Simpan status sebelumnya untuk deteksi perubahan
        bool previousSelected = subTree->item.selected;
        GuiCheckBox(checkBox, NULL, &subTree->item.selected);

        // PERBAIKAN: Update hanya jika ada perubahan status dan bukan dalam mode disabled
        if (subTree->item.selected != previousSelected && !ctx->disableGroundClick) {
            if (subTree->item.selected) {
                // Tambahkan ke selected list
                selectFile(ctx->fileManager, &subTree->item);
            } else {
                // Hapus dari selected list
                deselectFile(ctx->fileManager, &subTree->item);
            }
        }

        colX += checkboxWidth;
    }

    // PERBAIKAN: Warna text berdasarkan status selection
    Color textColor = DARKGRAY;

    DrawText(TextFormat("%s", item.name), colX + 8, rowY + 6, 10, textColor);
    colX += colWidths[0];

    DrawText(item.type == ITEM_FILE ? "file" : "folder", colX + 8, rowY + 6, 10, textColor);
    colX += colWidths[1];

    if (item.size < KB_SIZE) {
        DrawText(TextFormat("%d B", item.size), colX + 8, rowY + 6, 10, textColor);
    } else if (item.size < MB_SIZE) {
        DrawText(TextFormat("%.2f KB", ((float)item.size / KB_SIZE)), colX + 8, rowY + 6, 10, textColor);
    } else if (item.size < GB_SIZE) {
        DrawText(TextFormat("%.2f MB", ((float)item.size / MB_SIZE)), colX + 8, rowY + 6, 10, textColor);
    } else {
        DrawText(TextFormat("%.2f GB", ((float)item.size / GB_SIZE)), colX + 8, rowY + 6, 10, textColor);
    }

    colX += colWidths[2];

    struct tm *local = localtime(&item.updated_at);

    char buffer[100];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", local);

    DrawText(TextFormat("%s", buffer), colX + 8, rowY + 6, 10, textColor);
}

void drawTableHeader(Context *ctx, Body *body, float x, float y, float colWidths[]) {
    int fontSize = 10;
    int headerHeight = 30;

    float colX = x;

    DrawRectangleRec((Rectangle){x, y, colWidths[0] + colWidths[1] + colWidths[2] + colWidths[3] + (body->showCheckbox ? 28 : 0), headerHeight}, LIGHTGRAY);
    DrawRectangleLinesEx((Rectangle){x, y, colWidths[0] + colWidths[1] + colWidths[2] + colWidths[3] + (body->showCheckbox ? 28 : 0), headerHeight}, 1, DARKGRAY);

    if (body->showCheckbox) {
        Rectangle checkRect = {
            colX + 7,
            y + (headerHeight - 14) / 2,
            14, 14};

        bool previousSelectedAll = body->selectedAll;
        GuiCheckBox(checkRect, NULL, &body->selectedAll);

        // PERBAIKAN: Update hanya jika ada perubahan dan bukan dalam mode disabled
        if (body->selectedAll != previousSelectedAll && !ctx->disableGroundClick) {
            if (body->selectedAll) {
                selectAll(ctx->fileManager);
            } else {
                clearSelectedFile(ctx->fileManager);
            }
        }

        colX += 28;
    }

    DrawRectangleLines(colX, y, colWidths[0], headerHeight, DARKGRAY);
    DrawText("Name", colX + 8, y + 8, fontSize, DARKGRAY);
    colX += colWidths[0];

    DrawRectangleLines(colX, y, colWidths[1], headerHeight, DARKGRAY);
    DrawText("Type", colX + 8, y + 8, fontSize, DARKGRAY);
    colX += colWidths[1];

    DrawRectangleLines(colX, y, colWidths[2], headerHeight, DARKGRAY);
    DrawText("Size", colX + 8, y + 8, fontSize, DARKGRAY);
    colX += colWidths[2];

    DrawRectangleLines(colX, y, colWidths[3], headerHeight, DARKGRAY);
    DrawText("Modified Time", colX + 8, y + 8, fontSize, DARKGRAY);
}
