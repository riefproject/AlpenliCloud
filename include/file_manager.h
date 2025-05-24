#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "nbtree.h"
#include "item.h"
#include "stack.h"
#include "queue.h"

#define alloc(T) (T *)malloc(sizeof(T));

typedef struct FileManager {
  Tree root;              // root directory
  LinkedList trash;         // root trash (deleted files)
  Stack undo;             // stack for undo operations
  Stack redo;             // stack for redo operations

  Tree treeCursor;        // current tree cursor 
  // (tree dengan root adalah direktori saat ini)

  char* currentPath;       // queue for current path
  Queue copied;            // queue for copied items
  Queue cut;               // queue for cut items
  Queue temp;              // temporary queue for operations
  LinkedList selectedItem; // linkedlist for selected item
} FileManager;

// HELPER PROTOTYPE: Line 130

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void createFileManager(FileManager* fileManager);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void initFileManager(FileManager* fileManager);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
Tree loadTree(Tree tree, char* path);


/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
Item searchFile(FileManager* fileManager, char* path);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void createFile(FileManager* fileManager, ItemType type, char* name);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void deleteFile(FileManager* fileManager);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void renameFile(FileManager* fileManager, char* filePath, char* newName);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void recoverFile(FileManager* fileManager);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void copyFile(FileManager* fileManager);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void pasteFile(FileManager* fileManager);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void cutFile(FileManager* fileManager);


/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void undo(FileManager* fileManager);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void redo(FileManager* fileManager);


/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void selectFile(FileManager* fileManager, Item item);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void deselectFile(FileManager* fileManager, Item item);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void clearSelectedFile(FileManager* fileManager);

/*
====================================================================
  Helper Functions (PRIVATE)
====================================================================
*/

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void _moveToTrash(FileManager* fileManager, Tree itemTree);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void _deletePermanently(char* fullPath, ItemType type, char* name);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void _deleteSingleItem(char* fullPath, ItemType type, char* name);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void _copyFileContent(char* srcPath, char* destPath);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void _copyFolderRecursive(char* srcPath, char* destPath);
void _removeFromTrash(FileManager* fileManager, char* itemName);
void _addBackToTree(FileManager* fileManager, TrashItem* trashItem, char* recoverPath);
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

Tree getCurrentRoot(FileManager* fileManager);

char* getCurrentPath(Tree tree);

void goBack(FileManager* fileManager);

void goTo(FileManager* FileManager, Tree tree);

void sort_children(Tree* parent);

#endif // !FILE_MANAGER_H