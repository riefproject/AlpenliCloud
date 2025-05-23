#ifndef NAVBAR_H
#define NAVBAR_H

#include <stdbool.h>
#include "raylib.h"
#include "file_manager.h"
typedef struct Navbar {
    bool textboxPatheditMode;
    char textboxPath[1024];

    bool textboxSearcheditMode;
    char textboxSearch[1024];

    Rectangle currentZeroPosition;

    FileManager *fileManager;
} Navbar;

void createNavbar(Navbar *navbar);

// void updateNavbar(Navbar *navbar, Rectangle currentZeroPosition, FileManager *filemanager);
void updateNavbar(Navbar *navbar, Rectangle currentZeroPosition, FileManager *fileManager);

void drawNavbar(Navbar *navbar);

#endif