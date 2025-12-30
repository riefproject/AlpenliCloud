#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#else
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "win_utils.h"

/*
================================================================================
    PUBLIC WINDOWS FILE OPERATIONS
================================================================================
*/


int RemoveItemsRecurse(const char* folderPath) {
#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    char searchPath[MAX_PATH];
    char itemPath[MAX_PATH];

    snprintf(searchPath, MAX_PATH, "%s\\*", folderPath);

    HANDLE hFind = FindFirstFileA(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        DWORD err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND) {
            // Folder kosong mungkin, tetap lanjut hapus folder
            return RemoveDirectoryA(folderPath) ? 1 : 0;
        }
        printf("[LOG] Gagal akses folder %s (Error: %lu)\n", folderPath, err);
        return 0;
    }

    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            continue;

        snprintf(itemPath, MAX_PATH, "%s\\%s", folderPath, findData.cFileName);

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!RemoveItemsRecurse(itemPath)) {
                // Jika gagal hapus isi folder, tetap coba lanjut
                printf("[LOG] Gagal hapus subfolder %s\n", itemPath);
            }
        }
        else {
            if (!DeleteFileA(itemPath)) {
                printf("[LOG] Gagal hapus file: %s (Error: %lu)\n", itemPath, GetLastError());
            }
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);

    if (!RemoveDirectoryA(folderPath)) {
        printf("[LOG] Gagal hapus folder: %s (Error: %lu)\n", folderPath, GetLastError());
        return 0;
    }

    return 1;
#else
    DIR* dir = opendir(folderPath);
    if (!dir) {
        if (rmdir(folderPath) == 0) {
            return 1;
        }
        printf("[LOG] Gagal akses folder %s (Error: %d)\n", folderPath, errno);
        return 0;
    }

    struct dirent* entry = NULL;
    char itemPath[PATH_MAX];
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(itemPath, sizeof(itemPath), "%s/%s", folderPath, entry->d_name);

        struct stat st;
        if (lstat(itemPath, &st) != 0) {
            printf("[LOG] Gagal akses item %s (Error: %d)\n", itemPath, errno);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (!RemoveItemsRecurse(itemPath)) {
                printf("[LOG] Gagal hapus subfolder %s\n", itemPath);
            }
        }
        else {
            if (unlink(itemPath) != 0) {
                printf("[LOG] Gagal hapus file: %s (Error: %d)\n", itemPath, errno);
            }
        }
    }

    closedir(dir);

    if (rmdir(folderPath) != 0) {
        printf("[LOG] Gagal hapus folder: %s (Error: %d)\n", folderPath, errno);
        return 0;
    }

    return 1;
#endif
}


/*
================================================================================
    PUBLIC WINDOWS DIALOG FUNCTIONS
================================================================================
*/


int OpenWindowsFileDialog(char* filePath, int maxPathLength) {
#ifdef _WIN32
    OPENFILENAMEA ofn;
    char szFile[260] = { 0 };

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL; // Changed from GetActiveWindow() to NULL for compatibility
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All Files\0*.*\0Text Files\0*.TXT\0Document Files\0*.DOC;*.DOCX\0Image Files\0*.PNG;*.JPG;*.JPEG;*.BMP;*.GIF\0Video Files\0*.MP4;*.AVI;*.MKV;*.MOV\0Audio Files\0*.MP3;*.WAV;*.FLAC;*.AAC\0Archive Files\0*.ZIP;*.RAR;*.7Z\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = "Select File to Import";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;

    // Display the Open dialog box
    if (GetOpenFileNameA(&ofn) == TRUE) {
        strncpy(filePath, szFile, maxPathLength - 1);
        filePath[maxPathLength - 1] = '\0';
        printf("[LOG] File dialog - Selected: %s\n", filePath);
        return 1; // Success
    }
    else {
        DWORD error = CommDlgExtendedError();
        if (error != 0) {
            printf("[LOG] File dialog error: %lu\n", error);
        }
        else {
            printf("[LOG] File dialog cancelled by user\n");
        }
        return 0; // Cancel or error
    }
#elif defined(__APPLE__)
    if (filePath == NULL || maxPathLength <= 0) {
        return 0;
    }

    const char* script = "osascript -e 'POSIX path of (choose file with prompt \"Select File to Import\")'";
    FILE* pipe = popen(script, "r");
    if (!pipe) {
        printf("[LOG] File dialog gagal dibuka\n");
        return 0;
    }

    if (!fgets(filePath, maxPathLength, pipe)) {
        pclose(pipe);
        printf("[LOG] File dialog cancelled by user\n");
        return 0;
    }
    pclose(pipe);

    size_t len = strlen(filePath);
    while (len > 0 && (filePath[len - 1] == '\n' || filePath[len - 1] == '\r')) {
        filePath[--len] = '\0';
    }

    if (len == 0) {
        return 0;
    }

    printf("[LOG] File dialog - Selected: %s\n", filePath);
    return 1;
#else
    (void)filePath;
    (void)maxPathLength;
    printf("[LOG] File dialog tidak didukung pada platform ini\n");
    return 0;
#endif
}

int OpenWindowsFolderDialog(char* folderPath, int maxPathLength) {
#ifdef _WIN32
    BROWSEINFOA bi = { 0 };
    bi.hwndOwner = NULL; // Changed for compatibility
    bi.lpszTitle = "Select Folder to Import";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;

    // Initialize COM (required for modern folder dialog)
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);

    if (pidl != NULL) {
        // Get the path from the pidl
        if (SHGetPathFromIDListA(pidl, folderPath)) {
            // Free memory used by pidl
            IMalloc* imalloc = 0;
            if (SUCCEEDED(SHGetMalloc(&imalloc))) {
                imalloc->lpVtbl->Free(imalloc, pidl);
                imalloc->lpVtbl->Release(imalloc);
            }

            printf("[LOG] Folder dialog - Selected: %s\n", folderPath);

            // Uninitialize COM
            if (SUCCEEDED(hr)) {
                CoUninitialize();
            }
            return 1; // Success
        }

        // Free memory used by pidl
        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->lpVtbl->Free(imalloc, pidl);
            imalloc->lpVtbl->Release(imalloc);
        }
    }

    // Uninitialize COM
    if (SUCCEEDED(hr)) {
        CoUninitialize();
    }

    printf("[LOG] Folder dialog cancelled or failed\n");
    return 0; // Cancel or error
#elif defined(__APPLE__)
    if (folderPath == NULL || maxPathLength <= 0) {
        return 0;
    }

    const char* script = "osascript -e 'POSIX path of (choose folder with prompt \"Select Folder to Import\")'";
    FILE* pipe = popen(script, "r");
    if (!pipe) {
        printf("[LOG] Folder dialog gagal dibuka\n");
        return 0;
    }

    if (!fgets(folderPath, maxPathLength, pipe)) {
        pclose(pipe);
        printf("[LOG] Folder dialog cancelled by user\n");
        return 0;
    }
    pclose(pipe);

    size_t len = strlen(folderPath);
    while (len > 0 && (folderPath[len - 1] == '\n' || folderPath[len - 1] == '\r')) {
        folderPath[--len] = '\0';
    }

    if (len == 0) {
        return 0;
    }

    printf("[LOG] Folder dialog - Selected: %s\n", folderPath);
    return 1;
#else
    (void)folderPath;
    (void)maxPathLength;
    printf("[LOG] Folder dialog tidak didukung pada platform ini\n");
    return 0;
#endif
}


/*
================================================================================
    PUBLIC WINDOWS PATH FUNCTIONS
================================================================================
*/


int GetWindowsCommonPath(WindowsCommonFolder folder, char* path, int pathSize) {
#ifdef _WIN32
    char* userProfile = getenv("USERPROFILE");
    if (!userProfile) {
        printf("[LOG] USERPROFILE environment variable not found\n");
        return 0;
    }

    switch (folder) {
    case WIN_FOLDER_DESKTOP:
        snprintf(path, pathSize, "%s\\Desktop", userProfile);
        break;
    case WIN_FOLDER_DOCUMENTS:
        snprintf(path, pathSize, "%s\\Documents", userProfile);
        break;
    case WIN_FOLDER_DOWNLOADS:
        snprintf(path, pathSize, "%s\\Downloads", userProfile);
        break;
    case WIN_FOLDER_PICTURES:
        snprintf(path, pathSize, "%s\\Pictures", userProfile);
        break;
    case WIN_FOLDER_MUSIC:
        snprintf(path, pathSize, "%s\\Music", userProfile);
        break;
    case WIN_FOLDER_VIDEOS:
        snprintf(path, pathSize, "%s\\Videos", userProfile);
        break;
    default:
        printf("[LOG] Unknown folder type: %d\n", folder);
        return 0;
    }

    printf("[LOG] Common path generated: %s\n", path);
    return 1;
#else
    const char* home = getenv("HOME");
    const char* suffix = NULL;
    if (!home) {
        printf("[LOG] HOME environment variable not found\n");
        return 0;
    }

    switch (folder) {
    case WIN_FOLDER_DESKTOP:
        suffix = "Desktop";
        break;
    case WIN_FOLDER_DOCUMENTS:
        suffix = "Documents";
        break;
    case WIN_FOLDER_DOWNLOADS:
        suffix = "Downloads";
        break;
    case WIN_FOLDER_PICTURES:
        suffix = "Pictures";
        break;
    case WIN_FOLDER_MUSIC:
        suffix = "Music";
        break;
    case WIN_FOLDER_VIDEOS:
        suffix = "Movies";
        break;
    default:
        printf("[LOG] Unknown folder type: %d\n", folder);
        return 0;
    }

    if (home[strlen(home) - 1] == '/') {
        snprintf(path, pathSize, "%s%s", home, suffix);
    }
    else {
        snprintf(path, pathSize, "%s/%s", home, suffix);
    }

    printf("[LOG] Common path generated: %s\n", path);
    return 1;
#endif
}

/*
================================================================================
    PUBLIC HELPER FUNCTIONS - PATH UTILITIES
================================================================================
*/

/**
 * @brief Converts input path to full Windows absolute path
 *
 * Resolves relative paths, UNC paths, and path shortcuts to their full
 * absolute representation using Windows API. Dynamically allocates memory
 * to accommodate paths of any length up to system limits.
 *
 * @param[in] inputPath Input path to be converted (relative or absolute)
 *
 * @return char* Pointer to dynamically allocated full path string
 * @retval Non-NULL Success - returns pointer to full path string
 * @retval NULL Failure - invalid input path or memory allocation failed
 *
 * @pre inputPath must be a valid null-terminated string
 * @post If successful, returns dynamically allocated string with full path
 *
 * @note Uses GetFullPathNameA() Windows API for path resolution
 * @note Caller is responsible for freeing the returned memory using free()
 * @note Automatically handles buffer resizing for long paths
 *
 * @warning Returned pointer must be freed by caller to prevent memory leaks
 * @warning Returns NULL on failure - always check return value before use
 *
 * @internal This is a private helper function - not exposed in header file
 */
char* _getFullWindowsPath(const char* inputPath) {
#ifdef _WIN32
    DWORD bufferSize = MAX_PATH;
    char* fullPath = NULL;

    while (1) {
        fullPath = (char*)malloc(bufferSize);
        if (!fullPath)
            return NULL;

        DWORD len = GetFullPathNameA(inputPath, bufferSize, fullPath, NULL);
        if (len == 0) {
            // Error: gagal mendapatkan path
            free(fullPath);
            return NULL;
        }

        if (len < bufferSize) {
            // Berhasil
            return fullPath;
        }

        // Buffer terlalu kecil, alokasikan ulang dengan ukuran yang cukup
        free(fullPath);
        bufferSize = len + 1;
    }
#else
    if (inputPath == NULL) {
        return NULL;
    }

    char resolved[PATH_MAX];
    if (realpath(inputPath, resolved)) {
        return strdup(resolved);
    }

    if (inputPath[0] == '/') {
        return strdup(inputPath);
    }

    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        return NULL;
    }

    size_t len = strlen(cwd) + 1 + strlen(inputPath) + 1;
    char* fullPath = (char*)malloc(len);
    if (!fullPath) {
        return NULL;
    }

    snprintf(fullPath, len, "%s/%s", cwd, inputPath);
    return fullPath;
#endif
}

// Function untuk validasi path Windows
int ValidateWindowsPath(const char* path) {
#ifdef _WIN32
    if (!path || strlen(path) == 0) {
        return 0;
    }

    // Check if path exists
    DWORD dwAttrib = GetFileAttributesA(path);
    if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
        printf("[LOG] Path validation failed - not found: %s\n", path);
        return 0; // Path doesn't exist
    }

    // Check if we can access it (simple read test)
    HANDLE hFile = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) ? FILE_FLAG_BACKUP_SEMANTICS : FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("[LOG] Path validation failed - access denied: %s (Error: %lu)\n", path, GetLastError());
        return 0; // Can't access
    }

    CloseHandle(hFile);
    printf("[LOG] Path validation successful: %s\n", path);
    return 1; // Valid and accessible
#else
    if (!path || strlen(path) == 0) {
        return 0;
    }

    struct stat st;
    if (stat(path, &st) != 0) {
        printf("[LOG] Path validation failed - not found: %s (Error: %d)\n", path, errno);
        return 0;
    }

    if (access(path, R_OK) != 0) {
        printf("[LOG] Path validation failed - access denied: %s (Error: %d)\n", path, errno);
        return 0;
    }

    printf("[LOG] Path validation successful: %s\n", path);
    return 1;
#endif
}
