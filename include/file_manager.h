#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "nbtree.h"
#include "item.h"

typedef struct FileManager{
  // Tree root;
  // Tree rootTrash;
  // Stack undo, redo;
  // Queue selectedItem;
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