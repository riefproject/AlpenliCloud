#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "nbtree.h"
#include "item.h"
#include "stack.h"
#include "queue.h"
typedef struct FileManager {
  char *currentPath;
  Tree root;
  Tree rootTrash;
  Stack actionStack;
  Queue selectedItem;
} FileManager;

// OPERASI FILE
void initFileManager(FileManager *fileManager);

void createFile(FileManager *fileManager);
void deleteFile(FileManager *fileManager);
void updateFile(FileManager *fileManager);
void recoverFile(FileManager *fileManager);
void searchFile(FileManager *fileManager);

void undo(FileManager *fileManager);
void redo(FileManager *fileManager);

void copyFile(FileManager *fileManager);
void pasteFile(FileManager *fileManager);
void cutFile(FileManager *fileManager);

void printDirectory(FileManager *fileManager);
void printTrash(FileManager *fileManager);

#endif // !FILE_MANAGER_H