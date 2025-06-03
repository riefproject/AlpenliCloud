#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "raylib.h"
#include <stdbool.h>

/*
====================================================================
    FORWARD DECLARATIONS DAN ENUMERASI
====================================================================
*/

typedef struct Context Context;

typedef enum ResizeDirection {
    RESIZE_NONE,
    RESIZE_LEFT,
    RESIZE_RIGHT,
    RESIZE_TOP,
    RESIZE_BOTTOM
} ResizeDirection;

/*
====================================================================
    STRUKTUR DATA TITLEBAR
====================================================================
*/

typedef struct TitleBar {
    Vector2 mousePosition;
    Vector2 windowPosition;
    Vector2 panOffset;
    Vector2 resizeOrigin;

    int screenWidth;
    int screenHeight;

    int width;
    int height;

    bool dragWindow;
    bool exitWindow;
    bool resizing;

    bool isBottonMaximizeClicked;
    bool isBottonMinimizeClicked;

    ResizeDirection resizeDir;

    Context* ctx;
} TitleBar;

/*
====================================================================
    INISIALISASI DAN UPDATE TITLEBAR
====================================================================
*/

// Prosedur create titlebar component
// Menginisialisasi struktur TitleBar dengan nilai default dan referensi context
// IS: TitleBar pointer dan Context valid
// FS: TitleBar terinisialisasi dengan height 23, semua Vector2 diset (0,0), semua bool flag diset false, resizeDir RESIZE_NONE, screenWidth/screenHeight dari ctx, ctx direferensikan
// Created by: Farras
void createTitleBar(TitleBar* titleBar, Context* ctx);

// Prosedur update titlebar component
// Memperbarui state titlebar dan menangani window drag, resize, maximize, minimize berdasarkan mouse interaction
// IS: TitleBar dan Context valid, mouse input dapat dideteksi
// FS: mousePosition diupdate dari GetMousePosition, resize direction dideteksi dengan GetResizeDirection, cursor diupdate dengan UpdateResizeCursor, window dragging/resizing dihandle dengan SetWindowPosition/SetWindowSize, button maximize/minimize diproses dengan MaximizeWindow/MinimizeWindow/RestoreWindow, screenWidth/screenHeight diupdate dari GetScreenWidth/GetScreenHeight
// Created by: Farras
void updateTitleBar(TitleBar* titleBar, Context* ctx);

/*
====================================================================
    RENDERING DAN DRAWING
====================================================================
*/

// Prosedur draw titlebar component
// Menggambar window title bar dengan tombol maximize/minimize dan border resize
// IS: TitleBar valid dengan screenWidth dan screenHeight yang sudah diset
// FS: GuiWindowBox digambar dengan title "#198# PORTABLE WINDOW", tombol maximize (#198#) dan minimize (#35#) digambar dengan GuiButton, resize borders digambar di keempat sisi window dengan DrawRectangle dan warna GRAY fade, button style border width diatur
// Created by: Farras
void drawTitleBar(TitleBar* titleBar);

#endif