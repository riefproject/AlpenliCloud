#ifndef CTX_H
#define CTX_H

#include "raylib.h"

typedef struct FileManager FileManager;
typedef struct Rectangle Rectangle;
typedef struct Body Body;
typedef struct Navbar Navbar;
typedef struct Sidebar Sidebar;
typedef struct TitleBar TitleBar;
typedef struct Toolbar Toolbar;

typedef struct Context {
    FileManager *fileManager;
    Body *body;
    Navbar *navbar;
    Sidebar *sidebar;
    TitleBar *titleBar;
    Toolbar *toolbar;

    Rectangle *currentZeroPosition;

    int height;
    int width;
} Context;

void createContext(Context* ctx, FileManager *fileManager, int screenWidth, int screenHeight);
void updateContext(Context* ctx, FileManager *fileManager);

#endif // CTX_H
