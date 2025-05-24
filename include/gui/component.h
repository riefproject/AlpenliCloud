#ifndef COMPONENT_H
#define COMPONENT_H

#include "raylib.h"
#include "item.h"
#include "macro.h"
// #include "navbar.h"
// #include "sidebar.h"
// #include "titlebar.h"
// typedef struct Component {
//     Navbar *navbar;

// } Component;

typedef struct NewButtonProperty {
    Rectangle btnRect;
    Rectangle dropdownRect;
    Rectangle modalRect;

    char* placeholder;
    char* tooltip;
    char inputBuffer[MAX_STRING_LENGTH];

    int dropdownIndex;
    bool dropdownActive;

    ItemType selectedType;
    bool* showModal;
    bool* inputEditMode;

    bool disabled;
} NewButtonProperty;

bool GuiNewButton(NewButtonProperty* buttonProperty);

bool GuiTextBoxCustom(Rectangle bounds, char* icon, char* placeholder, char* inputText, int textSize, bool* editMode, bool disabled);

bool GuiButtonCustom(Rectangle bounds, const char* text, const char* tooltip, bool disabled);

#endif // COMPONENT_H