#ifndef COMPONENT_H
#define COMPONENT_H

#include "item.h"
#include "macro.h"
#include "sidebar.h"
// #include "navbar.h"
// #include "sidebar.h"
// #include "titlebar.h"
// typedef struct Component {
//     Navbar *navbar;

// } Component;

typedef struct ButtonWithModalProperty {
    Rectangle btnRect;      // Rectangle yang mendefinisikan posisi dan ukuran tombol utama
    Rectangle dropdownRect; // Rectangle yang mendefinisikan posisi dan ukuran menu dropdown
    Rectangle modalRect;    // Rectangle yang mendefinisikan posisi dan ukuran dialog modal

    char* placeholder;
    char* tooltip;
    char inputBuffer[MAX_STRING_LENGTH];

    int dropdownIndex;
    bool dropdownActive;

    ItemType selectedType;
    bool showModal;
    bool inputEditMode;

    bool itemCreated;

    bool disabled;
    Sidebar *sidebar;
} ButtonWithModalProperty;

void GuiNewButton(ButtonWithModalProperty *buttonProperty);

bool GuiTextBoxCustom(Rectangle bounds, char *icon, char *placeholder, char *inputText, int textSize, bool *editMode, bool disabled);

bool GuiButtonCustom(Rectangle bounds, const char *text, const char *tooltip, bool disabled);

#endif // COMPONENT_H