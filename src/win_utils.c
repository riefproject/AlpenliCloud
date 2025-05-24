#define WIN32_LEAN_AND_MEAN
#include "win_utils.h"
#include <stdio.h>
#include <windows.h>

int RemoveItemsRecurse(const char *folderPath) {
    WIN32_FIND_DATAA findData;
    char searchPath[MAX_PATH];
    char itemPath[MAX_PATH];

    snprintf(searchPath, MAX_PATH, "%s\\*", folderPath);

    HANDLE hFind = FindFirstFileA(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return 0;

    do {
        // Lewatin "." dan ".."
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            continue;

        snprintf(itemPath, MAX_PATH, "%s\\%s", folderPath, findData.cFileName);

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Kalau folder → panggil rekursif
            RemoveItemsRecurse(itemPath);
        } else {
            // Kalau file → hapus
            if (!DeleteFileA(itemPath)) {
                printf("Gagal hapus file: %s\n", itemPath);
            }
        }

    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);

    // Setelah semua isi dihapus, hapus foldernya sendiri
    if (!RemoveDirectoryA(folderPath)) {
        printf("Gagal hapus folder: %s\n", folderPath);
        return 0;
    }

    return 1;
}

void OpenWith(const char *path) {
    char absolutePath[MAX_PATH];

    if (_fullpath(absolutePath, path, MAX_PATH) == NULL) {
        fprintf(stderr, "Gagal mendapatkan path absolut.\n");
        return;
    }

    // cek
    if (!PathFileExists(absolutePath)) {
        fprintf(stderr, "File tidak ditemukan: %s\n", absolutePath);
        return;
    }

    HINSTANCE result = ShellExecute(
        NULL,
        "openas", // "openas" = dialog Open With
        absolutePath,
        NULL,
        NULL,
        SW_SHOWNORMAL);

    if ((int)result <= 32) {
        printf(stderr, "Gagal membuka file dengan 'Open With'. Error code: %ld\n", (long)result);
    }
}