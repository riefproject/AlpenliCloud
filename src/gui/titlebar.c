
#include "gui/titlebar.h"
#include "raygui.h"

void createTitleBar(TitleBar* titleBar, int screenWidth, int screenHeight) { 
    titleBar->screenWidth = screenWidth;
    titleBar->screenHeight = screenHeight;

    titleBar->height = 23;

    titleBar->mousePosition = (Vector2){0, 0};
    titleBar->windowPosition = (Vector2){0, 0};
    titleBar->panOffset = (Vector2){0, 0};
    titleBar->dragWindow = false;
    titleBar->exitWindow = false;
}

void updateTitleBar(TitleBar* titleBar) {
    titleBar->mousePosition = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !titleBar->dragWindow) {
        if (CheckCollisionPointRec(titleBar->mousePosition, (Rectangle){0, 0, titleBar->screenWidth, 20})) {
            titleBar->windowPosition = GetWindowPosition();
            titleBar->dragWindow = true;
            titleBar->panOffset = titleBar->mousePosition;
        }
    }

    if (titleBar->dragWindow) {
        titleBar->windowPosition.x += (titleBar->mousePosition.x - titleBar->panOffset.x);
        titleBar->windowPosition.y += (titleBar->mousePosition.y - titleBar->panOffset.y);

        SetWindowPosition((int) titleBar->windowPosition.x, (int) titleBar->windowPosition.y);

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            titleBar->dragWindow = false;
    }
}

void drawTitleBar(TitleBar* titleBar) {
    titleBar->exitWindow = GuiWindowBox((Rectangle){0, 0, titleBar->screenWidth, titleBar->screenHeight}, "#198# PORTABLE WINDOW");
}