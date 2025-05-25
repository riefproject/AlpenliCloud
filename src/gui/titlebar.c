#include "gui/titlebar.h"
#include "macro.h"
#include "raygui.h"
#include "gui/ctx.h"

#define RESIZE_BORDER 2
#define MIN_WIDTH 300
#define MIN_HEIGHT 200

ResizeDirection GetResizeDirection(Vector2 mouse, int width, int height) {
    if (mouse.x <= RESIZE_BORDER)
        return RESIZE_LEFT;
    if (mouse.x >= width - RESIZE_BORDER)
        return RESIZE_RIGHT;
    if (mouse.y <= RESIZE_BORDER)
        return RESIZE_TOP;
    if (mouse.y >= height - RESIZE_BORDER)
        return RESIZE_BOTTOM;
    return RESIZE_NONE;
}

void UpdateResizeCursor(ResizeDirection dir) {
    switch (dir) {
    case RESIZE_LEFT:
    case RESIZE_RIGHT:
        SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);
        break;
    case RESIZE_TOP:
    case RESIZE_BOTTOM:
        SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);
        break;
    default:
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        break;
    }
}

void createTitleBar(TitleBar *titleBar, Context *ctx) {
    titleBar->ctx = ctx;
    ctx->titleBar = titleBar;

    titleBar->screenWidth = ctx->width;
    titleBar->screenHeight = ctx->height;

    titleBar->height = 23;

    titleBar->mousePosition = (Vector2){0, 0};
    titleBar->windowPosition = (Vector2){0, 0};
    titleBar->panOffset = (Vector2){0, 0};
    titleBar->dragWindow = false;
    titleBar->exitWindow = false;
    titleBar->resizeDir = RESIZE_NONE;
    titleBar->resizing = false;
    titleBar->resizeOrigin = (Vector2){0, 0};
    titleBar->isBottonMaximizeClicked = false;
    titleBar->isBottonMinimizeClicked = false;
}

void updateTitleBar(TitleBar *titleBar, Context *ctx) {
    titleBar->ctx = ctx;

    titleBar->mousePosition = GetMousePosition();
    Vector2 mouse = titleBar->mousePosition;
    Vector2 winPos = GetWindowPosition();
    Vector2 screenMouse = {mouse.x + winPos.x, mouse.y + winPos.y};

    if (!titleBar->dragWindow && !titleBar->resizing && !IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        titleBar->resizeDir = GetResizeDirection(mouse, titleBar->screenWidth, titleBar->screenHeight);
        UpdateResizeCursor(titleBar->resizeDir);
    }

    if (titleBar->resizeDir != RESIZE_NONE && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        titleBar->resizing = true;
        titleBar->resizeOrigin = screenMouse;
    }

    if (titleBar->resizing) {
        Vector2 delta = {screenMouse.x - titleBar->resizeOrigin.x, screenMouse.y - titleBar->resizeOrigin.y};
        int newWidth = titleBar->screenWidth;
        int newHeight = titleBar->screenHeight;
        Vector2 newPos = winPos;

        switch (titleBar->resizeDir) {
        case RESIZE_LEFT:
            newWidth -= (int)delta.x;
            newPos.x += (int)delta.x;
            break;
        case RESIZE_RIGHT:
            newWidth += (int)delta.x;
            break;
        case RESIZE_TOP:
            newHeight -= (int)delta.y;
            newPos.y += (int)delta.y;
            break;
        case RESIZE_BOTTOM:
            newHeight += (int)delta.y;
            break;
        default:
            break;
        }

        if (newWidth >= MIN_WIDTH && newHeight >= MIN_HEIGHT) {
            SetWindowSize(newWidth, newHeight);
            SetWindowPosition((int)newPos.x, (int)newPos.y);
            titleBar->screenWidth = newWidth;
            titleBar->screenHeight = newHeight;
        }

        titleBar->resizeOrigin = screenMouse;

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            titleBar->resizing = false;
            titleBar->resizeDir = RESIZE_NONE;
        }
    }

    if (!titleBar->resizing && !titleBar->dragWindow && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mouse, (Rectangle){0, 0, titleBar->screenWidth, titleBar->height})) {
            titleBar->windowPosition = GetWindowPosition();
            titleBar->dragWindow = true;
            titleBar->panOffset = screenMouse;
        }
    }

    if (titleBar->dragWindow) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 newMouse = GetMousePosition();
            Vector2 newScreenMouse = {newMouse.x + winPos.x, newMouse.y + winPos.y};
            Vector2 delta = {newScreenMouse.x - titleBar->panOffset.x, newScreenMouse.y - titleBar->panOffset.y};
            SetWindowPosition((int)(titleBar->windowPosition.x + delta.x), (int)(titleBar->windowPosition.y + delta.y));
        } else {
            titleBar->dragWindow = false;
        }
    }

    if (titleBar->isBottonMaximizeClicked) {
        if (IsWindowMaximized())
            RestoreWindow();
        else
            MaximizeWindow();
        titleBar->isBottonMaximizeClicked = false;
    }

    if (titleBar->isBottonMinimizeClicked) {
        MinimizeWindow();
        titleBar->isBottonMinimizeClicked = false;
    }

    titleBar->screenWidth = GetScreenWidth();
    titleBar->screenHeight = GetScreenHeight();
}

void drawTitleBar(TitleBar *titleBar) {
    titleBar->exitWindow = GuiWindowBox((Rectangle){0, 0, titleBar->screenWidth, titleBar->screenHeight}, "#198# PORTABLE WINDOW");

    GuiSetStyle(BUTTON, BORDER_WIDTH, 1);
    if (GuiButton((Rectangle){titleBar->screenWidth - 20 * 2 - TINY_PADDING, 3, 18, 18}, "#198#")) {
        titleBar->isBottonMaximizeClicked = true;
    }
    if (GuiButton((Rectangle){titleBar->screenWidth - 20 * 3 - TINY_PADDING * 2, 3, 18, 18}, "#35#")) {
        titleBar->isBottonMinimizeClicked = true;
    }
    GuiSetStyle(BUTTON, BORDER_WIDTH, 2);

    DrawRectangle(0, 0, RESIZE_BORDER, titleBar->screenHeight, Fade(GRAY, 0.2f));                                     // Left
    DrawRectangle(titleBar->screenWidth - RESIZE_BORDER, 0, RESIZE_BORDER, titleBar->screenHeight, Fade(GRAY, 0.2f)); // Right
    DrawRectangle(0, 0, titleBar->screenWidth, RESIZE_BORDER, Fade(GRAY, 0.2f));                                      // Top
    DrawRectangle(0, titleBar->screenHeight - RESIZE_BORDER, titleBar->screenWidth, RESIZE_BORDER, Fade(GRAY, 0.2f)); // Bottom
}