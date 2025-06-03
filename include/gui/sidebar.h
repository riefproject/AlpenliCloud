#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "file_manager.h"
#include "raylib.h"

#define BASE_SIDEBAR_PANEL_CONTENT_WIDTH 150

/*
====================================================================
    FORWARD DECLARATIONS DAN STRUKTUR DATA
====================================================================
*/

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

    Context* ctx;
} Sidebar;

/*
====================================================================
    INISIALISASI DAN UPDATE SIDEBAR
====================================================================
*/

// Prosedur create sidebar component
// Menginisialisasi struktur Sidebar dengan nilai default dan referensi context
// IS: Sidebar pointer dan Context valid
// FS: Sidebar terinisialisasi dengan panelRec kosong, panelContentRec (0,0,160,340), panelView kosong, panelScroll (0,0), sidebarRoot NULL, ctx direferensikan
// Created by: Farras
void createSidebar(Sidebar* sidebar, Context* ctx);

// Prosedur update sidebar component
// Memperbarui ukuran dan posisi sidebar serta memuat sidebarRoot dari fileManager
// IS: Sidebar dan Context valid, fileManager dapat diakses
// FS: sidebarRoot dibuat dari getCurrentRoot jika masih NULL, currentZeroPosition diupdate dengan offset y dan height adjustment, panelRec dihitung dengan lebar 170 dan height berdasarkan currentZeroPosition
// Created by: Farras
void updateSidebar(Sidebar* sidebar, Context* ctx);

/*
====================================================================
    KONSTRUKTOR SIDEBAR ITEM
====================================================================
*/

// Function create sidebar item
// Membuat SidebarItem secara rekursif dari Tree dengan malloc dan inisialisasi expanded false
// IS: Tree dapat berupa NULL atau valid dengan struktur first_son dan next_brother
// FS: SidebarItem dibuat dengan malloc, tree direferensikan, isExpanded false, first_son dan next_brother dibuat rekursif, return pointer SidebarItem atau NULL jika tree NULL
// Created by: Farras
SidebarItem* crateSidebarItem(Tree tree);

/*
====================================================================
    RENDERING DAN DRAWING
====================================================================
*/

// Prosedur draw sidebar component
// Menggambar sidebar dengan scroll panel dan semua item folder dalam tree structure
// IS: Sidebar valid dengan sidebarRoot yang sudah diinisialisasi
// FS: GuiScrollPanel digambar dengan scissor mode, drawPos dihitung dari panelRec dengan scroll offset, drawSidebarItem dipanggil untuk render semua item, panelContentRec width dan height diupdate berdasarkan content
// Created by: Farras
void drawSidebar(Sidebar* sidebar);

// Prosedur draw sidebar item recursive
// Menggambar item sidebar secara rekursif dengan indentasi dan handling mouse interaction
// IS: SidebarItem node valid, posisi dan ukuran item ditentukan parameter
// FS: Item folder digambar dengan arrow icon (#115#/#116#), indentasi berdasarkan depth, mouse click dihandle untuk expand/collapse dan navigasi goTo, background BLUE untuk treeCursor aktif, scrollWidth diupdate berdasarkan content width, child items digambar rekursif jika expanded
// Created by: Farras
void drawSidebarItem(Sidebar* sidebar, SidebarItem* node, FileManager* fileManager, Vector2* pos, int depth, float width, float height, float* scrollWidth);

/*
====================================================================
    UTILITY FUNCTIONS
====================================================================
*/

// Function get max child label width
// Menghitung lebar maksimum label dari semua child nodes untuk layout sidebar
// IS: SidebarItem node, depth indentasi, dan textSize diketahui
// FS: Lebar maksimum label dihitung dengan traversal rekursif semua child nodes, return lebar maksimum dalam pixel
// Created by: Farras
int getMaxChildLabelWidth(SidebarItem* node, int depth, int textSize);

#endif