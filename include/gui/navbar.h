#ifndef NAVBAR_H
#define NAVBAR_H

#include <stdbool.h>
#include "raylib.h"
#include "file_manager.h"
#include "macro.h"

/*
====================================================================
    FORWARD DECLARATIONS DAN STRUKTUR DATA
====================================================================
*/

typedef struct Context Context;

typedef struct Navbar {
    bool textboxPatheditMode;
    char textboxPath[MAX_STRING_LENGTH];

    bool textboxSearcheditMode;
    char textboxSearch[MAX_STRING_LENGTH];

    bool shouldGoToPath;
    bool shouldSearch;

    bool isUndoButtonClicked;
    bool isRedoButtonClicked;
    bool isGoBackButtonClicked;

    Rectangle currentZeroPosition;

    Context* ctx;
} Navbar;

/*
====================================================================
    INISIALISASI DAN UPDATE NAVBAR
====================================================================
*/

// Prosedur create navbar component
// Menginisialisasi struktur Navbar dengan nilai default dan referensi context
// IS: Navbar pointer dan Context valid
// FS: Navbar terinisialisasi dengan semua flag button false, shouldGoToPath/shouldSearch false, textboxPath/textboxSearch kosong, editMode false, currentZeroPosition kosong, ctx direferensikan
// Created by: Farras
void createNavbar(Navbar* navbar, Context* ctx);

// Prosedur update navbar component
// Memperbarui state navbar dan menangani interaksi button serta navigasi path manual
// IS: Navbar dan Context valid, file manager dapat diakses
// FS: currentZeroPosition diupdate, navigasi path manual diproses dengan searchTree dan goTo, search string diproses, button undo/redo/goBack dihandle, textboxPath disinkronkan dengan currentPath saat tidak edit mode
// Created by: Farras
void updateNavbar(Navbar* navbar, Context* ctx);

/*
====================================================================
    RENDERING DAN DRAWING
====================================================================
*/

// Prosedur draw navbar component
// Menggambar navbar dengan button undo/redo/back, textbox path, textbox search, dan garis pembatas
// IS: Navbar valid dengan currentZeroPosition yang sudah diset
// FS: Button undo/redo/back digambar dengan GuiButtonCustom dan icon (#56#/#57#/#117#), textbox path dengan icon folder (#1#), textbox search dengan icon search (#42#) dan placeholder "Search Item", GuiLine digambar sebagai pembatas, layout responsif berdasarkan lebar layar
// Created by: Farras
void drawNavbar(Navbar* navbar);

#endif