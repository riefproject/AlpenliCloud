#ifndef SIDEBAR_H
#define SIDEBAR_H

#define BASE_SIDEBAR_PANEL_CONTENT_WIDTH 150

#include "file_manager.h"
#include "raylib.h"

typedef struct Context Context;

typedef struct SidebarItem SidebarItem;
typedef struct SidebarItem {
    Tree tree;
    bool isExpanded;
    SidebarItem *first_son;
    SidebarItem *next_brother;
} SidebarItem;

typedef struct SidebarState {
    Tree treeNode;
    bool isExpanded;
    struct SidebarState *next;
} SidebarState;

typedef struct Sidebar {
    Rectangle panelRec;        // adalah ukuran sidebar
    Rectangle panelContentRec; // adalah ukuran isi sidebar
    Rectangle panelView;       // adalah ukuran yang terlihat di sidebar (tinggi - tinggi scroll bawah dll)
    Vector2 panelScroll;       // adalah jauhnya scroll

    SidebarItem *sidebarRoot;
    Rectangle currentZeroPosition;

    Context *ctx;
} Sidebar;

void createSidebar(Sidebar *sidebar, Context *ctx);

SidebarItem *createSidebarItem(Tree root);

SidebarItem *createSidebarItemWithState(Tree root, SidebarState *stateList);

void collectSidebarState(SidebarItem *item, SidebarState **stateList);

bool getExpandedForTree(Tree tree, SidebarState *stateList);

void destroySidebarState(SidebarState *stateList);


void destroySidebarItem(SidebarItem **item);

void updateSidebar(Sidebar *sidebar, Context *ctx);

void drawSidebar(Sidebar *sidebar);

void drawSidebarItem(Sidebar *sidebar, SidebarItem *node, FileManager *fileManager, Vector2 *pos, int depth, float width, float height, float *scrollWidth);

int getMaxChildLabelWidth(SidebarItem *node, int depth, int textSize);

#endif
