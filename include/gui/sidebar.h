#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "raylib.h"

typedef struct Sidebar
{
    Rectangle panelRec;         // adalah ukuran sidebar
    Rectangle panelContentRec;  // adalah ukuran isi sidebar
    Rectangle panelView;        // adalah ukuran yang terlihat di sidebar (tinggi - tinggi scroll bawah dll)
    Vector2 panelScroll;        // adalah jauhnya scroll 

    Rectangle currentZeroPosition;
} Sidebar;

void createSidebar(Sidebar *sidebar);

void updateSidebar(Sidebar *sidebar, Rectangle currentZeroPosition);

void drawSidebar(Sidebar *sidebar);

#endif
