#ifndef NAVBAR_H
#define NAVBAR_H

#include <stdbool.h>
#include <string.h>

#include "gui/component.h"

typedef struct Navbar {
    bool textboxPatheditMode;
    char textboxPath[1024];

    bool textboxSearcheditMode;
    char textboxSearch[1024];

    Rectangle currentZeroPosition;
} Navbar;

void createNavbar(Navbar *navbar);

// void updateNavbar(Navbar *navbar, Rectangle currentZeroPosition, FileManager *filemanager);
void updateNavbar(Navbar *navbar, Rectangle currentZeroPosition);

void drawNavbar(Navbar *navbar);

#endif