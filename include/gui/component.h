#ifndef COMPONENT_H
#define COMPONENT_H

#include "raylib.h"
// #include "navbar.h"
// #include "sidebar.h"
// #include "titlebar.h"
// typedef struct Component {
//     Navbar *navbar;
    
// } Component;

bool GuiTextBoxCustom(Rectangle bounds, const char *placeholder, char *inputText, int textSize, bool *editMode, bool disabled);

bool GuiButtonCustom(Rectangle bounds, const char *text, const char *tooltip, bool disabled);

#endif // COMPONENT_H