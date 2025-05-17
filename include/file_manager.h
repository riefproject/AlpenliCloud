#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "nbtree.h"
#include "item.h"
#include "stack.h"
#include "queue.h"

typedef struct FileManager {
  Tree root;            // root directory
  Tree rootTrash;       // root trash (deleted files)   
  Stack undo;           // stack for undo operations
  Stack redo;           // stack for redo operations
  Queue currentPath;    // queue for current path       
  Queue copied;         // queue for copied items 
  Queue cut;            // queue for cut items
  Queue temp;           // temporary queue for operations
} FileManager;

// Create empty filemanager 
void createFileManager(FileManager* fileManager);

// Define filemanager with default value
void initFileManager(FileManager* fileManager, char* path);

void createFile(FileManager* fileManager);
void deleteFile(FileManager* fileManager);
void updateFile(FileManager* fileManager);
void recoverFile(FileManager* fileManager);
void searchFile(FileManager* fileManager);

void undo(FileManager* fileManager);
void redo(FileManager* fileManager);

void copyFile(FileManager* fileManager);
void pasteFile(FileManager* fileManager);
void cutFile(FileManager* fileManager);

void printDirectory(FileManager* fileManager);
void printTrash(FileManager* fileManager);

#endif // !FILE_MANAGER_H