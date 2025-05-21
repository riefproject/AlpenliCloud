

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "raylib.h"

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
} TitleBar;

void createTitleBar(TitleBar* titleBar, int screenWidth, int screenHeight);

void updateTitleBar(TitleBar* titleBar);

void drawTitleBar(TitleBar* titleBar);

#endif