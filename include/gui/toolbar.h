#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <stdbool.h>
#include "raylib.h"
#include "item.h"
#include "component.h"
#include "file_manager.h"
#include "sidebar.h"

/*
====================================================================
    FORWARD DECLARATIONS DAN STRUKTUR DATA
====================================================================
*/

typedef struct Context Context;

typedef struct Toolbar {
    ButtonWithModalProperty newButtonProperty;
    Rectangle currentZeroPosition;

    bool isButtonCopyActive;
    bool isButtonCutActive;
    bool isButtonDeleteActive;
    bool isButtonPasteActive;

    Context* ctx;
} Toolbar;

/*
====================================================================
    INISIALISASI DAN UPDATE TOOLBAR
====================================================================
*/

// Prosedur create toolbar component
// Menginisialisasi struktur Toolbar dengan nilai default dan konfigurasi newButtonProperty
// IS: Toolbar pointer dan Context valid
// FS: Toolbar terinisialisasi dengan newButtonProperty (btnRect 50,50,100,24; dropdownRect 50,82,100,60; modalRect 300,200,300,150; placeholder "#65# New"; tooltip "Add a file or folder"; selectedType ITEM_FILE), semua button flags false, currentZeroPosition kosong, ctx direferensikan
// Created by: Farras
void createToolbar(Toolbar* toolbar, Context* ctx);

// Prosedur update toolbar component
// Memperbarui posisi toolbar dan menangani aksi button yang aktif dengan file operations
// IS: Toolbar dan Context valid, fileManager dapat diakses
// FS: currentZeroPosition diupdate dengan offset y 24+DEFAULT_PADDING, btnRect dan dropdownRect diposisikan ulang, modalRect dipusatkan di layar, itemCreated diproses dengan createFile, button copy/cut/delete/paste diproses dengan operasi file yang sesuai dan flag direset
// Created by: Farras
void updateToolbar(Toolbar* toolbar, Context* ctx);

/*
====================================================================
    RENDERING DAN DRAWING
====================================================================
*/

// Prosedur draw toolbar component
// Menggambar toolbar dengan button New, Rename, Cut, Copy, Paste, Delete dan garis pembatas
// IS: Toolbar valid dengan currentZeroPosition yang sudah diset
// FS: Button New digambar dengan GuiNewButton, button Rename (#22#), Cut (#17#), Copy (#16#), Paste (#18#) digambar di kiri dengan spacing DEFAULT_PADDING, button Delete (#143#) digambar di kanan, GuiLine digambar sebagai pembatas bawah, layout responsif berdasarkan currentZeroPosition width
// Created by: Farras
void drawToolbar(Toolbar* toolbar);

#endif