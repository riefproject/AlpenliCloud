#ifndef COMPONENT_H
#define COMPONENT_H

#define DEFAULT_PADDING 10
#define TINY_PADDING 5

#include "raylib.h"
// #include "navbar.h"
// #include "sidebar.h"
// #include "titlebar.h"
// typedef struct Component {
//     Navbar *navbar;
    
// } Component;

bool GuiButtonTooltip(Rectangle bounds, const char *text, const char *tooltip);

#endif // COMPONENT_H