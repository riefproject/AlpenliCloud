#ifndef NAVBAR_H
#define NAVBAR_H

#include <stdbool.h>
#include "raylib.h"
#include "file_manager.h"
#include "macro.h"
typedef struct Navbar {
    bool textboxPatheditMode;
    char textboxPath[MAX_STRING_LENGTH];

    bool textboxSearcheditMode;
    char textboxSearch[MAX_STRING_LENGTH];

    Rectangle currentZeroPosition;

    FileManager *fileManager;
} Navbar;

void createNavbar(Navbar *navbar);

// void updateNavbar(Navbar *navbar, Rectangle currentZeroPosition, FileManager *filemanager);
void updateNavbar(Navbar *navbar, Rectangle currentZeroPosition, FileManager *fileManager);

void drawNavbar(Navbar *navbar);

#endif