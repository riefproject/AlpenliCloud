#ifndef WIN_UTILS_H
#define WIN_UTILS_H

// Enum untuk common Windows folders
typedef enum {
    WIN_FOLDER_DESKTOP,
    WIN_FOLDER_DOCUMENTS,
    WIN_FOLDER_DOWNLOADS,
    WIN_FOLDER_PICTURES,
    WIN_FOLDER_MUSIC,
    WIN_FOLDER_VIDEOS
} WindowsCommonFolder;

// Function declarations untuk Windows utilities
int RemoveItemsRecurse(const char* folderPath);

// File dialog functions
int OpenWindowsFileDialog(char* filePath, int maxPathLength);
int OpenWindowsFolderDialog(char* folderPath, int maxPathLength);

// Path utility functions
void GetWindowsUserFriendlyPath(const char* fullPath, char* result, int resultSize);
int GetWindowsCommonPath(WindowsCommonFolder folder, char* path, int pathSize);
int ValidateWindowsPath(const char* path);
int IsWindowsDirectory(const char* path);
void NormalizeWindowsPath(char* path);

// File system functions
long long GetWindowsFileSize(const char* filePath);

#endif // WIN_UTILS_H