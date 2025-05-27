#include <stdio.h>
#include <stdlib.h>

#include "file_manager.h"
#include "gui/ctx.h"
#include "gui/sidebar.h"
#include "macro.h"
#include "raygui.h"

void createSidebar(Sidebar *sidebar, Context *ctx) {
    sidebar->ctx = ctx;

    sidebar->panelRec = (Rectangle){0};
    sidebar->panelContentRec = (Rectangle){0, 0, 160, 340};
    sidebar->panelView = (Rectangle){0};
    sidebar->panelScroll = (Vector2){0};
    sidebar->sidebarRoot = NULL;
}

void updateSidebar(Sidebar *sidebar, Context *ctx) {
    sidebar->ctx = ctx;
    ctx->sidebar = sidebar;

    if (sidebar->sidebarRoot == NULL) {
        sidebar->sidebarRoot = crateSidebarItem(getCurrentRoot(sidebar->ctx->fileManager));
    }

    sidebar->currentZeroPosition = *ctx->currentZeroPosition;
    sidebar->currentZeroPosition.y = DEFAULT_PADDING * 3 + 24 * 3;
    sidebar->currentZeroPosition.height = +sidebar->currentZeroPosition.height - DEFAULT_PADDING * 2 - 24 * 2;

    sidebar->panelRec = (Rectangle){sidebar->currentZeroPosition.x, sidebar->currentZeroPosition.y, 170, sidebar->currentZeroPosition.height};
}

void drawSidebar(Sidebar *sidebar) {
    if (sidebar->sidebarRoot == NULL)
        return;

    float itemHeight = 24;
    Vector2 scroll = sidebar->panelScroll;

    if (sidebar->ctx->disableGroundClick)
        GuiDisable();
    GuiScrollPanel(sidebar->panelRec, NULL, sidebar->panelContentRec, &scroll, &sidebar->panelView);
    GuiEnable();

    sidebar->panelScroll = scroll;

    Vector2 drawPos = {
        sidebar->panelRec.x + DEFAULT_PADDING + scroll.x,
        sidebar->panelRec.y + DEFAULT_PADDING + scroll.y};

    float scrollWidth = sidebar->panelContentRec.width; // Initial width

    BeginScissorMode(sidebar->panelView.x, sidebar->panelView.y, sidebar->panelView.width, sidebar->panelView.height);

    drawSidebarItem(
        sidebar,
        sidebar->sidebarRoot,
        sidebar->ctx->fileManager,
        &drawPos,
        0,
        sidebar->panelContentRec.width,
        itemHeight,
        &scrollWidth);

    EndScissorMode();

    // Update width and height
    sidebar->panelContentRec.width = scrollWidth;
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

void drawSidebarItem(Sidebar *sidebar, SidebarItem *node, FileManager *fileManager, Vector2 *pos, int depth, float width, float height, float *scrollWidth) {
    while (node) {
        Tree itemNode = node->tree;

        if (itemNode->item.type == ITEM_FOLDER) {
            float indent = (DEFAULT_PADDING * 2) * depth;
            const char *arrow = (itemNode->first_son != NULL) ? (node->isExpanded ? "#116#" : "#115#") : "   ";
            const char *label = TextFormat("%s %s", arrow, itemNode->item.name);

            int labelTextWidth = MeasureText(label, GuiGetStyle(DEFAULT, TEXT_SIZE));
            float totalWidth = indent + labelTextWidth + DEFAULT_PADDING;

            if (totalWidth > *scrollWidth) {
                *scrollWidth = totalWidth;
            }

            Rectangle labelBounds = {pos->x + indent, pos->y, labelTextWidth, height};

            // Interaction
            if (!sidebar->ctx->disableGroundClick) {
                Vector2 mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse, labelBounds)) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        if (CheckCollisionPointRec(mouse, (Rectangle){labelBounds.x, labelBounds.y, 20, height}) && itemNode->first_son) {
                            node->isExpanded = !node->isExpanded;
                        } else {
                            goTo(fileManager, itemNode);
                            printf("Navigating to: %s\n", itemNode->item.name);
                        }
                    }
                }
            }

            if (itemNode == fileManager->treeCursor) {
                DrawRectangleRec((Rectangle){pos->x + indent, pos->y, *scrollWidth, height}, Fade(BLUE, 0.2f));
            }

            GuiLabel(labelBounds, label);
            pos->y += height;

            if (node->isExpanded && node->first_son) {
                drawSidebarItem(sidebar, node->first_son, fileManager, pos, depth + 1, width, height, scrollWidth);
            }
        }

        node = node->next_brother;
    }
}
