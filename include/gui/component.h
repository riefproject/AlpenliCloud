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

typedef Context Context;
typedef struct ButtonWithModalProperty {
    Rectangle btnRect;      // Rectangle yang mendefinisikan posisi dan ukuran tombol utama
    Rectangle dropdownRect; // Rectangle yang mendefinisikan posisi dan ukuran menu dropdown
    Rectangle modalRect;    // Rectangle yang mendefinisikan posisi dan ukuran dialog modal

    char *yesButtonText; // Teks untuk tombol "Ya" pada dialog modal
    char *noButtonText;  // Teks untuk tombol "Tidak" pada dialog modal
    char *title;         // Judul dialog modal

    char *placeholder;
    char *tooltip;
    char inputBuffer[MAX_STRING_LENGTH];

    int dropdownIndex;
    bool dropdownActive;

    ItemType selectedType;
    bool showModal;
    bool inputEditMode;

    bool itemCreated;

    bool disabled;
} ButtonWithModalProperty;

void GuiNewButton(ButtonWithModalProperty *buttonProperty, Context *ctx);
void DrawCreateModal(Context *ctx, ButtonWithModalProperty *buttonProperty);

bool GuiButtonCustom(Rectangle bounds, const char *text, const char *tooltip, bool disabled, bool notClickable);
bool GuiTextBoxCustom(Rectangle bounds, char *icon, char *placeholder, char *inputText, int textSize, bool *editMode, bool disabled, bool notClickable);
#endif // COMPONENT_H