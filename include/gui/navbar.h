#ifndef NAVBAR_H
#define NAVBAR_H

#include <stdbool.h>
#include "raylib.h"
#include "file_manager.h"
#include "macro.h"

typedef struct Context Context;

typedef struct Navbar {
    bool textboxPatheditMode;
    char textboxPath[MAX_STRING_LENGTH];

    bool textboxSearcheditMode;
    char textboxSearch[MAX_STRING_LENGTH];

    bool shouldGoToPath;  
    bool shouldSearch;    

    bool isUndoButtonClicked;
    bool isRedoButtonClicked;
    bool isGoBackButtonClicked;

    Rectangle currentZeroPosition;

    Context *ctx;
} Navbar;


void createNavbar(Navbar *navbar, Context *ctx);
void updateNavbar(Navbar *navbar, Context *ctx);
void drawNavbar(Navbar *navbar);

#endif