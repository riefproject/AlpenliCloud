#pragma once

/*
====================================================================
    OPERASI FILE SISTEM WINDOWS
====================================================================
*/

// Function remove items recursive
// Menghapus folder dan semua isinya secara rekursif menggunakan Windows API dengan FindFirstFile dan RemoveDirectory
// IS: Path folder yang valid dan dapat diakses
// FS: Semua file dalam folder dihapus dengan DeleteFileA, subfolder dihapus rekursif dengan RemoveItemsRecurse, folder utama dihapus dengan RemoveDirectoryA, return 1 jika berhasil atau 0 jika gagal
// Created by: Arief
int RemoveItemsRecurse(const char* path);