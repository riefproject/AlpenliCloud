#include "file_manager.h"
#include "stack.h"
#include "queue.h"
#include "nbtree.h"

void initFileManager(FileManager *fileManager){
  create_stack(&(fileManager->actionStack));
  create_queue(&(fileManager->selectedItem));
  create_tree(&(fileManager->root));
  create_tree(&(fileManager->rootTrash));
}

void createFile(FileManager *fileManager) {
    
}

void deleteFile(FileManager *fileManager) {}
void updateFile(FileManager *fileManager) {}
void recoverFile(FileManager *fileManager) {}
void searchFile(FileManager *fileManager) {}


void undo(FileManager *fileManager) {}
void redo(FileManager *fileManager) {}

void copyFile(FileManager *fileManager) {}
void pasteFile(FileManager *fileManager) {}
void cutFile(FileManager *fileManager) {}

void printDirectory(FileManager *fileManager) {}
void printTrash(FileManager *fileManager) {}
