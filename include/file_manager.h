#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "nbtree.h"
#include "item.h"
#include "stack.h"
#include "queue.h"

#define alloc(T) (T *)malloc(sizeof(T));

typedef struct FileManager {
  Tree root;      // root directory
  Tree rootTrash; // root trash (deleted files)
  Stack undo;     // stack for undo operations
  Stack redo;     // stack for redo operations

  Tree treeCursor;

  char* currentPath;  // queue for current path
  Queue copied;       // queue for copied items
  Queue cut;          // queue for cut items
  Queue temp;         // temporary queue for operations
  LinkedList selectedItem; // linkedlist for selected item
} FileManager;

// Create empty filemanager
void createFileManager(FileManager* fileManager);

// Define filemanager with default value
void initFileManager(FileManager* fileManager);

Tree loadTree(Tree tree, char* path);

#ifndef ITEM_OPERATION
#define ITEM_OPERATION
void createFile(FileManager* fileManager, ItemType type, char* name);
void deleteFile(FileManager* fileManager);
void renameFile(FileManager* fileManager, char* filePath, char* newName);
void recoverFile(FileManager* fileManager);
Item searchFile(FileManager* fileManager, char* path);

void copyFile(FileManager* fileManager);
void pasteFile(FileManager* fileManager);
void cutFile(FileManager* fileManager);
#endif
void undo(FileManager* fileManager);
void redo(FileManager* fileManager);
void selectFile(FileManager* fileManager, Item item);
void deselectFile(FileManager* fileManager, Item item);
void clearSelectedFile(FileManager* fileManager);

/*
====================================================================
  Helper Functions (PRIVATE)
==================================================================
*/
// Mengembalikan nama file dari path lengkap
char* _getNameFromPath(char* path);

/*
  Membuat nama folder baru dengan menambahkan suffix untuk folder duplikat
  suffix adalah string yang ditambahkan ke belakang nama folder
  Misal: "folder" menjadi "folder(1)"
*/
char* _createDuplicatedFolderName(char* dirPath, char* suffix);

/*
  Membuat nama file baru dengan menambahkan suffix untuk file duplikat
  suffix adalah string yang ditambahkan ke belakang nama file
  Misal: "file.txt" menjadi "file(1).txt"
*/
char* _createDuplicatedFileName(char* filePath, char* suffix);

// Cek apakah path adalah folder
bool isDirectory(char* path);

void windowsOpenWith(char* path);

char* getCurrentPath(Tree tree);

void goBack(FileManager* fileManager);

void goTo(FileManager* FileManager, Tree tree);

#endif // !FILE_MANAGER_H