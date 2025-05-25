// titlebar.h
#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "raylib.h"
#include <stdbool.h>

typedef enum ResizeDirection {
    RESIZE_NONE,
    RESIZE_LEFT,
    RESIZE_RIGHT,
    RESIZE_TOP,
    RESIZE_BOTTOM
} ResizeDirection;

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
} TitleBar;

void createTitleBar(TitleBar *titleBar, int screenWidth, int screenHeight);
void updateTitleBar(TitleBar *titleBar);
void drawTitleBar(TitleBar *titleBar);

#endif
