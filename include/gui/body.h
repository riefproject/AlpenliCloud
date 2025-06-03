#ifndef BODY_H
#define BODY_H

#include "raylib.h"
#include "file_manager.h"

/*
====================================================================
    FORWARD DECLARATIONS DAN STRUKTUR DATA
====================================================================
*/

typedef struct Context Context;

typedef struct Body {
    Rectangle panelRec;
    Rectangle panelContentRec;
    Rectangle panelView;
    Vector2 panelScroll;
    Rectangle currentZeroPosition;

    bool showCheckbox;

    bool selectedAll;
    bool previousSelectedAll;

    int focusedIndex;
    bool selected[100];

    Context* ctx;
} Body;

/*
====================================================================
    INISIALISASI DAN UPDATE BODY
====================================================================
*/

// Prosedur create body component
// Menginisialisasi struktur Body dengan nilai default dan mengatur referensi context
// IS: Context dan Body pointer telah dialokasikan
// FS: Body terinisialisasi dengan panelRec kosong, panelContentRec (0,0,170,340), panelScroll (0,0), focusedIndex -1, showCheckbox true, selectedAll false, ctx direferensikan
// Created by: 
void createBody(Context* ctx, Body* b);

// Prosedur update body component
// Memperbarui ukuran panel body dan status selectedAll berdasarkan state file manager
// IS: Context dan Body valid, fileManager->treeCursor dapat diakses
// FS: currentZeroPosition diupdate, panelRec dihitung ulang berdasarkan DEFAULT_PADDING, selectedAll diupdate berdasarkan ratio selected items terhadap total items
// Created by: 
void updateBody(Context* ctx, Body* body);

/*
====================================================================
    RENDERING DAN DRAWING
====================================================================
*/

// Prosedur draw body component
// Menggambar main body area dengan scroll panel, table header, dan semua item dalam tree cursor
// IS: Context dan Body valid, fileManager->treeCursor memiliki first_son
// FS: Children diurutkan dengan sort_children, GuiScrollPanel digambar dengan scissor mode, semua item ditampilkan dengan drawTableItem, header digambar dengan drawTableHeader, panelContentRec height disesuaikan dengan jumlah items
// Created by: 
void drawBody(Context* ctx, Body* body);

// Prosedur draw table item row
// Menggambar satu baris item dalam table dengan handling mouse interaction dan selection
// IS: Tree subTree valid, posisi dan ukuran row ditentukan oleh parameter
// FS: Row background digambar dengan warna berdasarkan status selected/focused, mouse click dihandle untuk selection/navigation (single/double click, Ctrl+click), checkbox dan text kolom digambar, gesture detection untuk double tap navigation
// Created by: 
void drawTableItem(Context* ctx, Body* body, Tree subTree, int index, float startX, float startY, float rowHeight, float colWidths[5]);

// Prosedur draw table header
// Menggambar header table dengan checkbox select all dan label kolom
// IS: Posisi header dan lebar kolom ditentukan oleh parameter
// FS: Header background LIGHTGRAY digambar dengan border, checkbox select all dirender dengan handling selectAll/clearSelectedFile, label kolom (Name, Type, Size, Modified Time) digambar dengan border DARKGRAY
// Created by: 
void drawTableHeader(Context* ctx, Body* body, float x, float y, float colWidths[]);

#endif