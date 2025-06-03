#ifndef COMPONENT_H
#define COMPONENT_H

#include "item.h"
#include "macro.h"
#include "sidebar.h"

/*
====================================================================
    FORWARD DECLARATIONS DAN STRUKTUR DATA
====================================================================
*/

typedef struct Context Context;

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

/*
====================================================================
    KOMPONEN BUTTON DAN MODAL
====================================================================
*/

// Prosedur new button dengan dropdown
// Menggambar tombol New dengan dropdown menu untuk File/Folder dan handling interaksi mouse
// IS: ButtonWithModalProperty dan Context valid
// FS: Tombol utama digambar dengan GuiButtonCustom, dropdown aktif menampilkan pilihan File/Folder dengan background dan border, klik File/Folder mengaktifkan modal, klik di luar area menutup dropdown
// Created by: Farras
void GuiNewButton(ButtonWithModalProperty* buttonProperty, Context* ctx);

// Prosedur draw create modal
// Menggambar modal dialog untuk membuat file/folder baru dengan input nama dan tombol Create/Cancel
// IS: Context valid, showModal true untuk menampilkan modal
// FS: Modal ditampilkan di tengah layar dengan overlay hitam, input box untuk nama item, tombol Create mengeksekusi pembuatan jika input tidak kosong, tombol Cancel atau ESC menutup modal
// Created by: Farras
void DrawCreateModal(Context* ctx);

/*
====================================================================
    KOMPONEN CUSTOM UI
====================================================================
*/

// Function custom button dengan tooltip
// Menggambar tombol custom dengan dukungan tooltip dan status disabled/notClickable
// IS: Rectangle bounds, text, tooltip, dan status button diketahui
// FS: Button digambar dengan style sesuai status (disabled dengan alpha rendah), tooltip ditampilkan saat hover, return true jika button ditekan dan tidak disabled
// Created by: Farras
bool GuiButtonCustom(Rectangle bounds, const char* text, const char* tooltip, bool disabled, bool notClickable);

// Function custom textbox dengan placeholder
// Menggambar textbox custom dengan dukungan icon, placeholder, dan mode edit/readonly
// IS: Rectangle bounds, icon, placeholder, inputText, dan status textbox diketahui
// FS: Textbox digambar dengan icon dan placeholder jika kosong, mode edit aktif saat diklik (jika tidak disabled), return true jika Enter ditekan dalam mode edit
// Created by: Farras
bool GuiTextBoxCustom(Rectangle bounds, char* icon, char* placeholder, char* inputText, int textSize, bool* editMode, bool disabled, bool notClickable);

/*
====================================================================
    PROGRESS BAR DAN STATE MANAGEMENT
====================================================================
*/

// Prosedur show paste progress bar
// Menampilkan modal progress bar untuk operasi paste dengan informasi progress dan item saat ini
// IS: currentProgress, totalItems, dan currentItemName diketahui
// FS: Modal progress ditampilkan di tengah layar dengan overlay, progress bar dengan persentase, informasi item yang sedang diproses, tombol Cancel untuk membatalkan operasi
// Created by: Arief
void showPasteProgressBar(int currentProgress, int totalItems, const char* currentItemName);

// Function check cancel paste operation
// Memeriksa apakah operasi paste harus dibatalkan berdasarkan state progressState
// IS: progressState.shouldCancel dapat diakses
// FS: Return true jika operasi harus dibatalkan, false jika operasi lanjut
// Created by: Arief
bool shouldCancelPaste();

// Prosedur reset progress bar state
// Mereset state progress bar ke kondisi awal (tidak aktif dan tidak cancel)
// IS: progressState dapat diakses
// FS: progressState.shouldCancel dan progressState.isActive diset false
// Created by: Arief
void resetProgressBarState();

#endif // COMPONENT_H