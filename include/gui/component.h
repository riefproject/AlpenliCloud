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
} ButtonWithModalProperty;


typedef struct {
    bool shouldCancel;
    bool isActive;
} ProgressBarState;

void GuiNewButton(ButtonWithModalProperty* buttonProperty, Context* ctx);
void DrawCreateModal(Context* ctx);

bool GuiButtonCustom(Rectangle bounds, const char* text, const char* tooltip, bool disabled, bool notClickable);
bool GuiTextBoxCustom(Rectangle bounds, char* icon, char* placeholder, char* inputText, int textSize, bool* editMode, bool disabled, bool notClickable);

/*  Prosedur untuk menampilkan progress bar paste operation
 *  IS: currentProgress dan totalItems diketahui
 *  FS: Progress bar ditampilkan di layar
================================================================================*/
void showPasteProgressBar(int currentProgress, int totalItems, const char* currentItemName);

bool shouldCancelPaste();

void resetProgressBarState();
#endif // COMPONENT_H