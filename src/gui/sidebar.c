#include <stdio.h>

#include "file_manager.h"
#include "gui/sidebar.h"
#include "macro.h"
#include "raygui.h"

void createSidebar(Sidebar *sidebar) {
    sidebar->panelRec = (Rectangle){0};
    sidebar->panelContentRec = (Rectangle){0, 0, 150, 340};
    sidebar->panelView = (Rectangle){0};
    sidebar->panelScroll = (Vector2){0};
    sidebar->sidebarRoot = NULL;
}

void updateSidebar(Sidebar *sidebar, Rectangle currentZeroPosition, FileManager *fileManager) {
    sidebar->fileManager = fileManager;

    if (sidebar->sidebarRoot == NULL) {
        sidebar->sidebarRoot = crateSidebarItem(getCurrentRoot(sidebar->fileManager));
    }

    sidebar->currentZeroPosition = currentZeroPosition;
    sidebar->currentZeroPosition.y = DEFAULT_PADDING * 3 + 24 * 3;
    sidebar->currentZeroPosition.height = +sidebar->currentZeroPosition.height - DEFAULT_PADDING * 2 - 24 * 2;

    sidebar->panelRec = (Rectangle){sidebar->currentZeroPosition.x, sidebar->currentZeroPosition.y, 170, sidebar->currentZeroPosition.height};
}

void drawSidebar(Sidebar *sidebar) {
    if (sidebar->sidebarRoot == NULL)
        return;

    float maxWidth = sidebar->panelRec.width;
    float itemHeight = 24;

    Vector2 scroll = sidebar->panelScroll;

    GuiScrollPanel(sidebar->panelRec, NULL, sidebar->panelContentRec, &scroll, &sidebar->panelView);
    sidebar->panelScroll = scroll;

    // Hitung posisi awal gambar
    Vector2 drawPos = {
        sidebar->panelRec.x + DEFAULT_PADDING + scroll.x,
        sidebar->panelRec.y + DEFAULT_PADDING + scroll.y};

    float scrollWidth = sidebar->panelContentRec.width; // nilai awal
    BeginScissorMode(sidebar->panelView.x, sidebar->panelView.y, sidebar->panelView.width, sidebar->panelView.height);

    drawSidebarItem(sidebar->sidebarRoot, &drawPos, 0, sidebar->panelContentRec.width, itemHeight, &scrollWidth);

    EndScissorMode();

    // Update panel content width jika lebih panjang dari sebelumnya
    if (scrollWidth > sidebar->panelContentRec.width) {
        sidebar->panelContentRec.width = scrollWidth + DEFAULT_PADDING;
    }

    // Update height juga kalau perlu
    sidebar->panelContentRec.height = drawPos.y - sidebar->panelRec.y;
}

SidebarItem *crateSidebarItem(Tree tree) {
    if (tree == NULL)
        return NULL;

    SidebarItem *sidebarItem = malloc(sizeof(SidebarItem));
    sidebarItem->tree = tree;
    sidebarItem->isExpanded = false;
    sidebarItem->first_son = crateSidebarItem(tree->first_son);
    sidebarItem->next_brother = crateSidebarItem(tree->next_brother);
    return sidebarItem;
}

void drawSidebarItem(SidebarItem *node, Vector2 *pos, int depth, float width, float height, float *scrollWidth) {
    while (node) {
        Tree itemNode = node->tree;

        if (itemNode->item.type == ITEM_FOLDER) {
            float indent = (DEFAULT_PADDING * 2) * depth;
            const char *arrow = (itemNode->first_son != NULL) ? (node->isExpanded ? "#116#" : "#115#") : "   ";
            const char *label = TextFormat("%s %s", arrow, itemNode->item.name);

            // Hitung panjang label jika ditampilkan
            int iconWidth = MeasureText(arrow, GuiGetStyle(DEFAULT, TEXT_SIZE));
            int labelWidth = MeasureText(label, GuiGetStyle(DEFAULT, TEXT_SIZE)) + indent + DEFAULT_PADDING;

            // Selalu update scrollWidth dengan label node ini
            if (labelWidth > *scrollWidth) {
                *scrollWidth = labelWidth;
            }

            // Rectangle bounds = ;

            // Toggle expand/collapse
            if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){pos->x + indent, pos->y, iconWidth, height}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (itemNode->first_son != NULL) {
                    node->isExpanded = !node->isExpanded;
                }
            }

            // Draw item
            GuiPanel((Rectangle){pos->x + indent, pos->y, labelWidth - indent, height}, NULL);
            GuiPanel((Rectangle){pos->x + indent, pos->y, iconWidth, height}, NULL);
            GuiLabel((Rectangle){pos->x + indent, pos->y, labelWidth - indent, height}, label);
            pos->y += height;

            // Jika di-expand, rekursi untuk anak, dan hitung lebar anak-anak juga
            if (node->isExpanded && node->first_son) {
                drawSidebarItem(node->first_son, pos, depth + 1, width, height, scrollWidth);
            }
        }

        node = node->next_brother;
    }
}
