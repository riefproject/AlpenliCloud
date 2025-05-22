#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "nbtree.h"
#include "item.h"
#include "stack.h"
#include "queue.h"

#define alloc(T) (T*)malloc(sizeof(T));


typedef struct FileManager {
  Tree root;            // root directory
  Tree rootTrash;       // root trash (deleted files)   
  Stack undo;           // stack for undo operations
  Stack redo;           // stack for redo operations
  char* currentPath;    // queue for current path       
  Queue copied;         // queue for copied items 
  Queue cut;            // queue for cut items
  Queue temp;           // temporary queue for operations
  Queue selectedItem;    // queue for selected item
} FileManager;

// Create empty filemanager 
void createFileManager(FileManager* fileManager);

// Define filemanager with default value
void initFileManager(FileManager* fileManager);

void createFile(FileManager* fileManager, ItemType type, char* name);
void deleteFile(FileManager* fileManager);
void renameFile(FileManager* fileManager, char* filePath, char* newName);
void recoverFile(FileManager* fileManager);
Item searchFile(FileManager* fileManager, char* path);

void undo(FileManager* fileManager);
void redo(FileManager* fileManager);

void copyFile(FileManager* fileManager);
void pasteFile(FileManager* fileManager);
void cutFile(FileManager* fileManager);
void selectFile(FileManager* fileManager, Item item);
void deselectFile(FileManager* fileManager, Item item);
void clearSelectedFile(FileManager* fileManager);


// Mengembalikan nama file dari path lengkap
char* getNameFromPath(char* path);

char* createDuplicatedFolderName(char* dirPath, char* suffix);
char* createDuplicatedFileName(char* filePath, char* suffix);
// Cek apakah path adalah folder
bool isDirectory(char* path);

#endif // !FILE_MANAGER_H