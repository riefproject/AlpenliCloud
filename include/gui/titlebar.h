

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "raylib.h"


typedef enum ResizeDirection {
    RESIZE_NONE = 0,
    RESIZE_LEFT,
    RESIZE_RIGHT,
    RESIZE_TOP,
    RESIZE_BOTTOM
} ResizeDirection;
typedef struct TitleBar {
    Vector2 mousePosition;
    Vector2 windowPosition;
    Vector2 panOffset;

    int screenWidth;
    int screenHeight;

    int height;
    int width;
    
    bool dragWindow;
    bool exitWindow;
    bool resizing;

    enum ResizeDirection resizeDir;
    Vector2 prevMouse;
    Vector2 resizeOrigin;

} TitleBar;

ResizeDirection GetResizeDirection(Vector2 mouse, int width, int height);

void UpdateResizeCursor(ResizeDirection dir);

void createTitleBar(TitleBar* titleBar, int screenWidth, int screenHeight);

void updateTitleBar(TitleBar* titleBar);

void drawTitleBar(TitleBar* titleBar);

#endif