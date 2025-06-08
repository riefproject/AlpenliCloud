#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "component.h"
#include "file_manager.h"
#include "item.h"
#include "raylib.h"
#include "sidebar.h"
#include <stdbool.h>

#define MAX_FILEPATH_COUNT 16

/*
====================================================================
    FORWARD DECLARATIONS DAN STRUKTUR DATA
====================================================================
*/

typedef struct Context Context;

typedef struct Toolbar {
    Rectangle currentZeroPosition;

    // untuk button
    bool isButtonCopyClicked;
    bool isButtonCutClicked;
    bool isButtonDeleteClicked;
    bool isButtonPasteClicked;
    bool isbuttonRenameClicked;
    bool isButtonRestoreClicked;
    bool isButtonPermanentDeleteClicked;
    bool isButtonImportClicked;
    // Import modal properties
    bool importClicked;
    bool showImportModal;
    bool importModalResult;
    char importPath[512];

    bool isButtonCreateItemClicked;

    // untuk create item
    bool showCreateModal;
    bool createItemModalResult;
    bool dropdownActive;

    bool inputCreateItemEditMode;
    char inputCreateItemBuffer[MAX_STRING_LENGTH];

    ItemType selectedType;

    // untuk rename item
    bool showRenameModal;
    bool renameModalResult;
    char renameInputBuffer[MAX_STRING_LENGTH];

    bool showImportModal;
    bool importModalResult;

    // --- State untuk Drag & Drop ---
    char droppedPaths[MAX_FILEPATH_COUNT][512];
    int droppedPathCount;

    // --- State untuk Input Manual ---
    char importPath[512];
    bool pathEditMode;

    Context *ctx;
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
void createToolbar(Toolbar *toolbar, Context *ctx);

// Prosedur update toolbar component
// Memperbarui posisi toolbar dan menangani aksi button yang aktif dengan file operations
// IS: Toolbar dan Context valid, fileManager dapat diakses
// FS: currentZeroPosition diupdate dengan offset y 24+DEFAULT_PADDING, btnRect dan dropdownRect diposisikan ulang, modalRect dipusatkan di layar, itemCreated diproses dengan createFile, button copy/cut/delete/paste diproses dengan operasi file yang sesuai dan flag direset
// Created by: Farras
void updateToolbar(Toolbar *toolbar, Context *ctx);

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
void drawToolbar(Toolbar *toolbar);

// Prosedur draw new button dropdown
// Menggambar dropdown menu untuk tombol New dengan pilihan File/Folder
// IS: Context valid, toolbar dropdown
// FS: Dropdown digambar di bawah tombol New dengan background dan border, pilihan File/Folder digambar sebagai button, klik pada pilihan mengaktifkan modal create item, klik di luar area dropdown menutup dropdown
// Created by: Farras
void DrawNewButtonDropdown(Context *ctx, Rectangle btnRect);

// Prosedur draw new item modal
// Menggambar modal dialog untuk membuat item baru (file/folder) dengan input nama dan tombol Create/Cancel
// IS: Context valid, toolbar showCreate
// FS: Modal ditampilkan di tengah layar dengan overlay, input box untuk nama item, tombol Create mengeksekusi pembuatan item jika nama valid, tombol
// Cancel atau ESC menutup modal, inputCreateItemBuffer direset setelah Create
// Created by: Farras
void DrawNewItemModal(Context *ctx);

// Prosedur draw rename item modal
// Menggambar modal dialog untuk mengubah nama item yang dipilih dengan input nama baru dan tombol Rename/Cancel
// IS: Context valid, toolbar showRenameModal
// FS: Modal ditampilkan di tengah layar dengan overlay, input box untuk nama baru, tombol Rename mengeksekusi pengubahan nama jika input valid, tombol
// Cancel atau ESC menutup modal, renameInputBuffer direset setelah Rename
// Created by: Farras
void DrawRenameItemModal(Context *ctx);

// Prosedur draw import modal
// Menggambar modal dialog untuk mengimpor file/folder dengan input path dan tombol Import/Cancel
// IS: Context valid, showImportModal true untuk menampilkan modal
// FS: Modal ditampilkan di tengah layar dengan overlay, input box untuk path import, tombol Import mengeksekusi import jika path valid, tombol Cancel atau ESC menutup modal
// Created by: GitHub Copilot
void DrawImportModal(Context *ctx);

#endif