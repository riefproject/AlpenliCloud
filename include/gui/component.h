#ifndef COMPONENT_H
#define COMPONENT_H

#include "item.h"
#include "macro.h"
#include "raylib.h"
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

    char *placeholder;                   // Teks placeholder yang ditampilkan ketika tidak ada input
    char *tooltip;                       // Teks tooltip yang ditampilkan saat hover
    char inputBuffer[MAX_STRING_LENGTH]; // Buffer untuk menyimpan teks input pengguna
    ItemType selectedType;               // Tipe item yang sedang dipilih dari dropdown

    int dropdownIndex;   // Indeks item yang sedang dipilih pada dropdown
    bool dropdownActive; // Penanda apakah menu dropdown sedang terbuka

    bool showModal;     // Penanda untuk menampilkan/menyembunyikan dialog modal
    bool inputEditMode; // Penanda apakah field input sedang dalam mode edit

    bool itemCreated; // Penanda apakah item baru telah dibuat

    bool disabled; // Penanda untuk mengaktifkan/menonaktifkan komponen tombol
} ButtonWithModalProperty;

void GuiNewButton(ButtonWithModalProperty *buttonProperty);

bool GuiTextBoxCustom(Rectangle bounds, char *icon, char *placeholder, char *inputText, int textSize, bool *editMode, bool disabled);

bool GuiButtonCustom(Rectangle bounds, const char *text, const char *tooltip, bool disabled);

#endif // COMPONENT_H