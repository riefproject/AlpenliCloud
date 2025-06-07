#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ctx.h"
#include "file_manager.h"
#include "gui/component.h"
#include "gui/navbar.h"
#include "gui/sidebar.h"
#include "macro.h"
#include "raygui.h"

// SidebarItem Management
SidebarItem *createSidebarItem(Tree root) {
    if (root == NULL)
        return NULL;

    SidebarItem *sidebarItem = malloc(sizeof(SidebarItem));
    sidebarItem->tree = root;
    sidebarItem->isExpanded = false;
    sidebarItem->first_son = createSidebarItem(root->first_son);
    sidebarItem->next_brother = createSidebarItem(root->next_brother);
    return sidebarItem;
}

SidebarItem *createSidebarItemWithState(Tree root, SidebarState *stateList) {
    if (root == NULL)
        return NULL;

    SidebarItem *sidebarItem = malloc(sizeof(SidebarItem));
    sidebarItem->tree = root;
    sidebarItem->isExpanded = getExpandedForTree(root, stateList);
    sidebarItem->first_son = createSidebarItemWithState(root->first_son, stateList);
    sidebarItem->next_brother = createSidebarItemWithState(root->next_brother, stateList);
    return sidebarItem;
}

void destroySidebarItem(SidebarItem **item) {
    if (item == NULL || *item == NULL)
        return;

    destroySidebarItem(&(*item)->first_son);
    destroySidebarItem(&(*item)->next_brother);
    free(*item);
    *item = NULL;
}

void collectSidebarState(SidebarItem *item, SidebarState **stateList) {
    if (item == NULL)
        return;

    SidebarState *state = malloc(sizeof(SidebarState));
    state->treeNode = item->tree;
    state->isExpanded = item->isExpanded;
    state->next = *stateList;
    *stateList = state;

    collectSidebarState(item->first_son, stateList);
    collectSidebarState(item->next_brother, stateList);
}

bool getExpandedForTree(Tree tree, SidebarState *stateList) {
    while (stateList) {
        if (stateList->treeNode == tree)
            return stateList->isExpanded;
        stateList = stateList->next;
    }
    return false; // default
}

void destroySidebarState(SidebarState *stateList) {
    SidebarState *tmp;
    while (stateList) {
        tmp = stateList;
        stateList = stateList->next;
        free(tmp);
    }
}

// Sidebar lifecycle
void createSidebar(Sidebar *sidebar, Context *ctx) {
    sidebar->ctx = ctx;
    sidebar->panelRec = (Rectangle){0};
    sidebar->panelContentRec = (Rectangle){0, 0, 160, 340};
    sidebar->panelView = (Rectangle){0};
    sidebar->panelScroll = (Vector2){0};
    sidebar->sidebarRoot = createSidebarItem(getCurrentRoot(*ctx->fileManager));
    sidebar->isButtonGoBackClicked = false;
    sidebar->isButtonOpenTrashClicked = false;
}

void updateSidebar(Sidebar *sidebar, Context *ctx) {
    sidebar->ctx = ctx;
    ctx->sidebar = sidebar;

    sidebar->currentZeroPosition = *ctx->currentZeroPosition;
    sidebar->currentZeroPosition.y = DEFAULT_PADDING * 3 + 24 * 3;
    sidebar->currentZeroPosition.height = sidebar->currentZeroPosition.height - DEFAULT_PADDING * 3 - 24 * 2 - 24 * 2;

    sidebar->panelRec = (Rectangle){
        sidebar->currentZeroPosition.x,
        sidebar->currentZeroPosition.y,
        170,
        sidebar->currentZeroPosition.height};

    if (sidebar->isButtonOpenTrashClicked) {
        sidebar->isButtonOpenTrashClicked = false;

        clearSelectedFile(sidebar->ctx->fileManager);
        strcpy(sidebar->ctx->navbar->textboxPath, "trash");
        sidebar->ctx->fileManager->currentPath = "trash";
        sidebar->ctx->fileManager->isRootTrash = true;
        printf("[LOG] Opening Trash...\n");
    }

    if (sidebar->isButtonGoBackClicked) {
        sidebar->isButtonGoBackClicked = false;

        clearSelectedFile(sidebar->ctx->fileManager);
        strcpy(sidebar->ctx->navbar->textboxPath, "root");
        sidebar->ctx->fileManager->currentPath = "root";
        sidebar->ctx->fileManager->isRootTrash = false;
        printf("[LOG] Going back to root...\n");
    }
}

void drawSidebar(Sidebar *sidebar) {
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

    float scrollWidth = sidebar->panelContentRec.width;
    sidebar->panelContentRec.width = scrollWidth + DEFAULT_PADDING;
    BeginScissorMode(sidebar->panelView.x, sidebar->panelView.y, sidebar->panelView.width, sidebar->panelView.height);

    if (sidebar->sidebarRoot) {
        drawSidebarItem(
            sidebar,
            sidebar->sidebarRoot,
            sidebar->ctx->fileManager,
            &drawPos,
            0,
            sidebar->panelContentRec.width,
            itemHeight,
            &scrollWidth);
    }

    EndScissorMode();

    sidebar->panelContentRec.width = scrollWidth;
    sidebar->panelContentRec.height = drawPos.y - sidebar->panelRec.y;

    if (!sidebar->ctx->fileManager->isRootTrash) {
        int prevBgColor = GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL);
        int prevTextColor = GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL);
        int prevBorderColor = GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL);

        int prevFocusedBgColor = GuiGetStyle(DEFAULT, BASE_COLOR_FOCUSED);
        int prevFocusedTextColor = GuiGetStyle(DEFAULT, TEXT_COLOR_FOCUSED);
        int prevFocusedBorderColor = GuiGetStyle(DEFAULT, BORDER_COLOR_FOCUSED);
        
        int prevPressedBgColor = GuiGetStyle(DEFAULT, BASE_COLOR_PRESSED);
        int prevPressedTextColor = GuiGetStyle(DEFAULT, TEXT_COLOR_PRESSED);
        int prevPressedBorderColor = GuiGetStyle(DEFAULT, BORDER_COLOR_PRESSED);

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

        sidebar->isButtonOpenTrashClicked = GuiButtonCustom(
            (Rectangle){
                sidebar->panelRec.x,
                sidebar->panelRec.y + sidebar->panelRec.height + DEFAULT_PADDING,
                sidebar->panelRec.width,
                24},
            "#143# Open Trash", NULL, false, sidebar->ctx->disableGroundClick);

        // Restore
        GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, prevBgColor);
        GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, prevTextColor);
        GuiSetStyle(DEFAULT, BORDER_COLOR_NORMAL, prevBorderColor);
        GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, prevFocusedBgColor);
        GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, prevFocusedTextColor);
        GuiSetStyle(DEFAULT, BORDER_COLOR_FOCUSED, prevFocusedBorderColor);
        GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, prevPressedBgColor);
        GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, prevPressedTextColor);
        GuiSetStyle(DEFAULT, BORDER_COLOR_PRESSED, prevPressedBorderColor);
    } else {
        sidebar->isButtonGoBackClicked = GuiButtonCustom(
            (Rectangle){
                sidebar->panelRec.x,
                sidebar->panelRec.y + sidebar->panelRec.height + DEFAULT_PADDING,
                sidebar->panelRec.width,
                24},
            "#72# Kembali", NULL, false, sidebar->ctx->disableGroundClick);
    }
}

void drawSidebarItem(Sidebar *sidebar, SidebarItem *node, FileManager *fileManager, Vector2 *pos, int depth, float width, float height, float *scrollWidth) {
    while (node) {
        Tree itemNode = node->tree;

        if (itemNode->item.type == ITEM_FOLDER) {
            float indent = (DEFAULT_PADDING * 2) * depth;
            const char *arrow = node->isExpanded ? "#116#" : "#115#";
            const char *label = TextFormat("%s %s", arrow, itemNode->item.name);

            int labelTextWidth = MeasureText(label, GuiGetStyle(DEFAULT, TEXT_SIZE));
            float totalWidth = indent + labelTextWidth + DEFAULT_PADDING;

            if (totalWidth > *scrollWidth) {
                *scrollWidth = totalWidth;
            }

            Rectangle labelBounds = {pos->x + indent, pos->y, *scrollWidth, height};

            // Interaction
            if (!sidebar->ctx->disableGroundClick) {
                Vector2 mouse = GetMousePosition();
                if (CheckCollisionPointRec(mouse, labelBounds)) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        if (CheckCollisionPointRec(mouse, (Rectangle){labelBounds.x, labelBounds.y, 20, height}) && itemNode->first_son) {
                            node->isExpanded = !node->isExpanded;
                        } else {
                            goTo(sidebar->ctx->fileManager, itemNode);
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
