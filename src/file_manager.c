#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "file_manager.h"
#include "stack.h"
#include "queue.h"
#include "nbtree.h"
#include "utils.h"
#include "item.h"

/* IS: 
  Tree root = ?;
  Tree rootTrash = ?;
  Stack actionStack = ?;
  Queue selectedItem = ?;
  Queue currentPath = ?;

  FS: 
  Tree root = NULL;
  Tree rootTrash = NULL;
  Stack actionStack = NULL;
  Queue selectedItem = NULL;
  Queue currentPath = NULL;
*/
void createFileManager(FileManager *fileManager) {
  create_stack(&(fileManager->actionStack));
  create_queue(&(fileManager->selectedItem));
  create_tree(&(fileManager->root));
  create_tree(&(fileManager->rootTrash));
}

/* 
  IS: 
  Tree root = NULL;
  Tree rootTrash = NULL;
  Stack actionStack = NULL;
  Queue selectedItem = NULL;
  Queue currentPath = NULL;

  FS: 
  Tree root = loadFile(*filemanager, "./root/dir/");
  Tree rootTrash = loadFile(*filemanager, "./root/trash/");
  Stack actionStack = NULL;
  Queue selectedItem = NULL;
  Queue currentPath = enqueue("./root/");
*/
void initFileManager(FileManager *fileManager) {
  printf("Hello World!");
  
}

int loadFile(FileManager *fileManager, char *path) {
  DIR *dp;
  struct dirent *ep;

  dp = opendir(path);

  if (dp != NULL) {
    while ((ep = readdir(dp)) != NULL) {
      Item * data = createItem(ep->d_name, 0, ITEM_FILE, NULL, NULL, NULL);
      insert_node(fileManager->root, data);
    }
    closedir(dp);
    return 0;
  } else {
    perror("Couldn't open the directory");
    return -1;
  }
}

void createFile(FileManager *fileManager) {
  int choice;
  char *fileName = NULL;
  char filepath[512];
  time_t createTime;

  while (true) {
    system("cls"); 
    printf("New Item\n");
    printf("Pilih type:\n");
    printf("1. File\n");
    printf("2. Folder\n");
    printf("Pilih type (1/2): ");
    scanf("%d", &choice);
    getchar();

    if (choice == 1 || choice == 2) break;
    printf("Pilihan tidak valid. Tekan enter untuk lanjut...\n");
    getchar();
  }

  printf("Masukkan nama %s: ", (choice == 1) ? "file" : "folder");
  inputString(&fileName);

  snprintf(filepath, sizeof(filepath), "%s/%s", fileManager->currentPath, fileName);

  if (choice == 1) {
 
    FILE *newFile = fopen(filepath, "w");
    if (newFile == NULL) {
      perror("Gagal membuat file");
      free(fileName);
      return;
    }
    fclose(newFile);

    // dapatkan waktu saat file dibuat
    createTime = time(NULL);


    Node *newNode = create_node((treeInfotype){
      .name = strdup(fileName),
      .size = 0,
      .type = isDirectory(filepath) ? ITEM_FOLDER : ITEM_FILE,
      .created_at = createTime,
      .updated_at = NULL,
      .deleted_at = NULL,
    });
    insert_node(fileManager->root, newNode);

    printf("File berhasil dibuat. Tekan enter untuk lanjut...\n");
    getchar();
  } else if (choice == 2) {
    int status = mkdir(filepath, 0755);
  
    if (status != 0) {
      perror("Gagal membuat folder");
      free(fileName);
      return;
    }

    Node *newNode = create_node((treeInfotype){
      .name = strdup(fileName),
      .size = 0,
      .type = ITEM_FOLDER,
      .created_at = NULL,
      .updated_at = NULL,
      .deleted_at = NULL,
    });
    insert_node(fileManager->root, newNode);

    printf("Folder berhasil dibuat. Tekan enter untuk lanjut...\n");
    getchar();
  }

  free(fileName);
}


void deleteFile(FileManager *fileManager) {}

// Rename/Update file name
void renameFile(FileManager *fileManager,char* filePath, char *newName) {
  Item item;

  // Cari item
  item = searchFile(fileManager, filePath);
  if(item == NULL) {
    printf("File tidak ditemukan\n");
    return;
  }

  // rename file
  char newPath[512];
  snprintf(newPath, sizeof(newPath), "%s/%s", fileManager->currentPath, newName);
  rename(filePath, newPath);
  
  // update item
  item.name = strdup(newName);
  item.path = strdup(newPath);
  
  printf("File berhasil diubah namanya menjadi %s\n", newName);
}

void recoverFile(FileManager *fileManager) {}

Item searchFile(FileManager *fileManager, char* path) {
  Item item, itemToSearch;
  itemToSearch = createItem(getNameFromPath(path), path,0, ITEM_FILE, NULL, NULL, NULL);
  item = searchTree(fileManager->root, item)->item;
  return item;
}


void undo(FileManager *fileManager) {}
void redo(FileManager *fileManager) {}

void copyFile(FileManager *fileManager) {}
void cutFile(FileManager *fileManager) {}
void pasteFile(FileManager *fileManager) {}

void printDirectory(FileManager *fileManager) {}
void printTrash(FileManager *fileManager) {}


char* getNameFromPath(char* path){
  char *name = strrchr(path, '/'); // dapatkan string yang dimulai dari karakter slash (/) terakhir
  if (name != NULL) {
    return name + 1; // skip karakter slash (/) terakhir
  }
  return path; // kembalikan pathnya kalau gak ada slash (/) (ini berarti sudah nama file)

};

bool isDirectory(char* path) {
  struct stat path_stat;
  stat(path, &path_stat);
  return S_ISDIR(path_stat.st_mode);
}