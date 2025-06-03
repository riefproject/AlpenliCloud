#ifndef UTILS_H
#define UTILS_H

/*
====================================================================
    FORWARD DECLARATIONS
====================================================================
*/

typedef struct Context Context;

/*
====================================================================
    UTILITAS INPUT DAN STRING
====================================================================
*/

// Prosedur input string dinamis
// Membaca input string dari user secara dinamis dengan realokasi memori hingga enter ditekan
// IS: Pointer string belum terinisialisasi atau kosong
// FS: String dialokasikan secara dinamis, diisi dengan input user karakter per karakter menggunakan realloc, diakhiri null terminator
// Created by: 
void inputString(char** s);

// Prosedur trim trailing slash
// Menghapus karakter '/' di akhir path string secara iteratif
// IS: String path mungkin memiliki satu atau lebih karakter '/' di akhir
// FS: Semua karakter '/' di akhir string dihapus, panjang string disesuaikan dengan null terminator
// Created by: 
void trimTrailingSlash(char* path);

/*
====================================================================
    KEYBOARD SHORTCUTS DAN NAVIGASI
====================================================================
*/

// Prosedur shortcut keys handler
// Menangani semua keyboard shortcuts untuk operasi file manager dan navigasi UI menggunakan raylib
// IS: Context berisi state aplikasi dan komponen UI yang valid
// FS: Shortcut keyboard diproses (Ctrl+C/V/X untuk copy/paste/cut, F5 untuk refresh, arrow keys untuk navigasi, dll), state UI dan file manager diperbarui sesuai shortcut
// Created by: 
void ShortcutKeys(Context* ctx);

#endif