#include "footer.h"
#include "ctx.h"
#include "file_manager.h"
#include "item.h"
#include "macro.h"
#include "raygui.h"
#include <stdio.h>

#define FOOTER_HEIGHT 24

void createFooter(Footer *footer, Context *ctx) {
    if (!footer || !ctx)
        return;

    footer->ctx = ctx;
    footer->bounds = (Rectangle){0, ctx->height - FOOTER_HEIGHT, ctx->width, FOOTER_HEIGHT};
    footer->dropdownBounds = (Rectangle){
        footer->bounds.x + 10,
        footer->bounds.y - 150,
        320,
        0 // dynamic
    };
    footer->hover = false;
}

void updateFooter(Footer *footer, Context *ctx) {
    if (!footer || !ctx)
        return;

    footer->ctx = ctx;
    footer->bounds = (Rectangle){
        .x = 0,
        .y = (float)(ctx->height - 24),
        .width = (float)ctx->width,
        .height = 24};
}

void drawFooter(Footer *footer) {
    if (!footer || !footer->ctx)
        return;

    FileManager *fm = footer->ctx->fileManager;
    Node *node = fm->selectedItem.head;

    int selectedCount = 0;
    long totalSize = 0;
    while (node) {
        Item *it = (Item *)node->data;
        selectedCount++;
        totalSize += it->size;
        node = node->next;
    }

    char text1[64];
    snprintf(text1, sizeof(text1), "%d item%s selected", selectedCount, selectedCount != 1 ? "s" : "");

    char text2[64];
    snprintf(text2, sizeof(text2), "%.2f KB", totalSize / 1024.0);
    if (totalSize < KB_SIZE) {
        snprintf(text2, sizeof(text2), "%ld B", totalSize);
    } else if (totalSize < MB_SIZE) {
        snprintf(text2, sizeof(text2), "%.2f KB", totalSize / 1024.0);
    } else if (totalSize < GB_SIZE) {
        snprintf(text2, sizeof(text2), "%.2f MB", totalSize / (float)MB_SIZE);
    } else {
        snprintf(text2, sizeof(text2), "%.2f GB", totalSize / (float)GB_SIZE);
    }

    // Background putih & border abu
    DrawRectangleRec(footer->bounds, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    DrawRectangleLinesEx(footer->bounds, 1, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

    // Text layout
    int padding = 8;
    int spacing = 12;
    int labelHeight = 22;

    int x = (int)footer->bounds.x + padding;
    int y = (int)(footer->bounds.y + (footer->bounds.height - labelHeight) / 2);

    Rectangle r1 = {x, y, MeasureTextEx(GuiGetFont(), text1, GuiGetStyle(DEFAULT, TEXT_SIZE), 1).x + 8, labelHeight};
    GuiLabel(r1, text1);
    x += r1.width + spacing;

    Rectangle r2 = {x, y, 12, labelHeight};
    GuiLabel(r2, "|");
    x += r2.width + spacing;

    Rectangle r3 = {x, y, MeasureTextEx(GuiGetFont(), text2, GuiGetStyle(DEFAULT, TEXT_SIZE), 1).x + 8, labelHeight};
    GuiLabel(r3, text2);
    x += r3.width + spacing;
}
