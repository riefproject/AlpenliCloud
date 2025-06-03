#ifndef CTX_H
#define CTX_H

#include "raylib.h"

/*
====================================================================
    FORWARD DECLARATIONS
====================================================================
*/

typedef struct FileManager FileManager;
typedef struct Body Body;
typedef struct Navbar Navbar;
typedef struct Sidebar Sidebar;
typedef struct TitleBar TitleBar;
typedef struct Toolbar Toolbar;

/*
====================================================================
    STRUKTUR DATA CONTEXT
====================================================================
*/

typedef struct Context {
    FileManager* fileManager;
    Body* body;
    Navbar* navbar;
    Sidebar* sidebar;
    TitleBar* titleBar;
    Toolbar* toolbar;

    Rectangle* currentZeroPosition;

    int height;
    int width;

    bool disableGroundClick;
} Context;

/*
====================================================================
    MANAJEMEN CONTEXT
====================================================================
*/

// Prosedur create context
// Menginisialisasi Context dengan alokasi semua komponen GUI dan pengaturan nilai awal
// IS: Context pointer dan FileManager valid, screenWidth dan screenHeight diketahui
// FS: Context terinisialisasi dengan ukuran layar, currentZeroPosition dialokasikan dengan Rectangle berdasarkan DEFAULT_PADDING, semua komponen GUI (titleBar, toolbar, navbar, sidebar, body) dialokasikan dan diinisialisasi, disableGroundClick diset false
// Created by: Farras
void createContext(Context* ctx, FileManager* fileManager, int screenWidth, int screenHeight);

// Prosedur update context
// Memperbarui Context dengan ukuran layar terbaru dan update semua komponen GUI
// IS: Context dan FileManager valid, layar mungkin telah berubah ukuran
// FS: currentZeroPosition diupdate berdasarkan GetScreenWidth/Height dan titleBar height, width/height Context diperbarui, fileManager direferensikan ulang, semua komponen GUI diupdate dengan urutan titleBar→navbar→toolbar→sidebar→body
// Created by: Farras
void updateContext(Context* ctx, FileManager* fileManager);

#endif // CTX_H