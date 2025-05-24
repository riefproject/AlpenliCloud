#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "file_action.h"
#include "file_manager.h"
#include "item.h"
#include "nbtree.h"
#include "queue.h"
#include "raylib.h"
#include "stack.h"
#include "utils.h"
#include "win_utils.h"
#include <string.h>
#include <time.h>
#define ROOT ".dir/root"

bool isCopy = 0;
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
    create_tree(&(fileManager->root));
    create_tree(&(fileManager->rootTrash));
    create_stack(&(fileManager->undo));
    create_stack(&(fileManager->redo));
    create_queue(&(fileManager->copied));
    create_queue(&(fileManager->cut));
    create_queue(&(fileManager->temp));
    create_list(&(fileManager->selectedItem));
}

void initFileManager(FileManager *fileManager) {
    Item rootItem;
    if (fileManager->root == NULL) {
        rootItem = createItem("root", ROOT, 0, ITEM_FOLDER, 0, 0, 0);
        fileManager->root = create_node_tree(rootItem);
        fileManager->currentPath = "root";

        fileManager->treeCursor = fileManager->root;

        loadTree(fileManager->treeCursor, ROOT);
        printf("\n\n");
    }
}

/* IS:
  Tree root = NULL;
  Tree rootTrash = NULL;
  Stack actionStack = NULL;
  Queue selectedItem = NULL;
  Queue currentPath = NULL;

  FS:
  Tree root = loadTree(*filemanager, "./root/dir/");
  Tree rootTrash = loadTree(*filemanager, "./root/trash/");
  Stack actionStack = NULL;
  Queue selectedItem = NULL;
  Queue currentPath = enqueue("./root/");
*/

// void initFileManager(FileManager* fileManager) {
//   printf("Hello World!");

// }
Tree loadTree(Tree tree, char *path) {
    DIR *dp;
    struct dirent *ep;
    struct stat statbuf;

    dp = opendir(path);
    if (dp == NULL) {
        perror("Tidak dapat membuka direktori");
        return NULL;
    }

    while ((ep = readdir(dp)) != NULL) {
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;

        char *fullPath = malloc(strlen(path) + strlen(ep->d_name) + 2);
        sprintf(fullPath, "%s/%s", path, ep->d_name);

        if (stat(fullPath, &statbuf) == -1) {
            perror("Gagal mendapatkan info file");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            Item data = createItem(ep->d_name, fullPath, statbuf.st_size, ITEM_FOLDER, statbuf.st_ctime, statbuf.st_mtime, 0);
            Tree newTree = insert_node(tree, data);
            loadTree(newTree, fullPath);
        } else if (S_ISREG(statbuf.st_mode)) {
            Item data = createItem(ep->d_name, fullPath, statbuf.st_size, ITEM_FILE, statbuf.st_ctime, statbuf.st_mtime, 0);
            insert_node(tree, data);
        } else {
            printf("  (Tipe lain) Ditemukan: %s\n", fullPath);
        }

        free(fullPath);
    }

    closedir(dp);
    return 0;
}

void createFile(FileManager *fileManager, ItemType type, char *name) {
    Item newItem, parentToSearch;
    char *path;
    Tree currentNode;
    time_t createdTime;
    FILE *newFile;

    currentNode = searchTree(fileManager->root, createItem(_getNameFromPath(fileManager->currentPath), fileManager->currentPath, 0, ITEM_FOLDER, 0, 0, 0));
    // printf("%s\n", fileManager->currentPath);
    if (currentNode != NULL) {
        path = TextFormat("%s/%s", fileManager->currentPath, name);
        createdTime = time(NULL);
        newItem = createItem(name, path, 0, type, createdTime, createdTime, -1);
        // printf("%s", path);
        if (type == ITEM_FOLDER) {
            if (DirectoryExists(path)) {
                path = _createDuplicatedFolderName(path, "(1)");
            }
            if (MakeDirectory(path) != 0) {
                printf("Gagal membuat folder\n");
                return;
            }
        } else if (type == ITEM_FILE) {
            if (FileExists(path)) {
                path = _createDuplicatedFileName(path, "(1)");
            }
            newFile = fopen(path, "w");
            if (newFile == NULL) {
                printf("Gagal membuat file %s\n", name);
                return;
            }
            fclose(newFile);
        }

        insert_node(currentNode, newItem);
    } else {
        printf("Direktori parent tidak ditemukan");
    }
}

void deleteFile(FileManager *fileManager) {
}

// Rename/Update file name
void renameFile(FileManager *fileManager, char *filePath, char *newName) {
    Item item;

    // Cari item
    item = searchFile(fileManager, filePath);
    // if (item == NULL) { // reviewed by Arief: bang struct itu ga bisa null, cheking null hanya untuk pointer cmiiw
    //   printf("File tidak ditemukan\n");
    //   return;
    // }

    // rename file
    char newPath[512];
    snprintf(newPath, sizeof(newPath), "%s/%s", fileManager->currentPath, newName);
    rename(filePath, newPath);

    // update item
    item.name = strdup(newName);
    item.path = strdup(newPath);

    printf("File berhasil diubah namanya menjadi %s\n", newName);
}

void recoverFile(FileManager *fileManager) {
}

Item searchFile(FileManager *fileManager, char *path) {
    Item item = {0};
    Item itemToSearch;
    Tree foundTree;

    itemToSearch = createItem(_getNameFromPath(path), path, 0, ITEM_FILE, 0, 0, 0);

    foundTree = searchTree(fileManager->root, itemToSearch);

    if (foundTree == NULL) {
        printf("File tidak ditemukan\n");
        return item;
    }
    item = foundTree->item;
    return item;
}

void undo(FileManager *fileManager) {
    //     if (fileManager->undo == NULL) {
    //         printf("No actions to undo.\n");
    //         return;
    //     }

    //     Action* action = (Action*)malloc(sizeof(Action));
    //     pop(&(fileManager->undo), action);
    //     push(&(fileManager->redo), action);
    //     switch (action->type) {
    //     case ACTION_MOVE:
    //     {
    //         FileManager* fm = fileManager;
    //         fm->treeCursor = searchTree(fm->root, createItem(_getNameFromPath(action->src), action->src, 0, ITEM_FOLDER, 0, 0, 0));

    //     }
    //     break;
    //     case ACTION_DELETE:
    //         break;
    //     case ACTION_RENAME:
    //         rename(action->dst, action->src);
    //         break;
    //     case ACTION_CREATE:
    //         if (action->isDir) {
    //             rmdir(action->src);
    //         }
    //         else {
    //             remove(action->src);
    //         }
    //         break;
    //     }
}
void redo(FileManager *fileManager) {
}

/*
  typedef struct FileManager {
    Tree root;
    Tree rootTrash;
    Stack undo;
    Stack redo;
    Queue selectedItem;
    Queue currentPath;
} FileManager;
*/
void copyFile(FileManager *fileManager) {
    // 1. deteksi file dipilih
    // 1. Cari item di tree
    // 2. Masukkan file terpillih satu per satu ke dalam queue
    // 3. a: Jika tidak ada, tampilkan pesan error
    //    b: Lanjut ke langkah 4
    // 4. Simpan di buffer
    // 5. tampilkan pesan error
    // 6. tampilkan pesan sukses
    if (fileManager->copied.front)
        fileManager->copied.front = NULL;
    Node *temp = fileManager->selectedItem.head;
    while (temp != NULL) {
        Item *itemToCopy = (Item *)temp->data;
        enqueue(&(fileManager->copied), itemToCopy);
        temp = temp->next;
    }
    isCopy = true;
    if (fileManager->copied.front == NULL) {
        printf("Gagal Menyalin File!\n");
        return;
    }
    fileManager->temp = fileManager->copied;
    printf("File berhasil disalin ke clipboard\n");
}

void cutFile(FileManager *fileManager) {
    if (fileManager->copied.front)
        fileManager->copied.front = NULL;
    Node *temp = fileManager->selectedItem.head;
    while (temp != NULL) {
        Item *itemToCopy = (Item *)temp->data;
        enqueue(&(fileManager->copied), itemToCopy);
        temp = temp->next;
    }
    isCopy = false;
    if (fileManager->copied.front == NULL) {
        printf("Gagal Menyalin File!\n");
        return;
    }
    fileManager->temp = fileManager->copied;
    printf("File berhasil disalin ke clipboard\n");
}

void pasteFile(FileManager *fileManager) {
    // 1. Ambil item yang dipilih dari queue satu per satu (iterasi sampai NULL)
    // 2. Cari item di tree
    // 3. a: Jika tidak ada, tampilkan pesan error
    //    b: Lanjut ke langkah 4
    // 4. Simpan di buffer
    // 5. Cari path di tree.
    // 6. a: Jika tidak ada, buatkan foldernya
    //    b: Jika ada, lanjutkan
    // 7. Simpan item ke path yang sudah ada
    // 8. Simpan item ke tree
    // 9. Hapus item dari queue
    // 10. Simpan semua operasi di stack undo
    // 11. Jika ada error, tampilkan pesan error
    // 12. Jika berhasil, tampilkan pesan sukses
    if (fileManager->temp.front == NULL) {
        printf("Clipboard kosong\n");
        return;
    }
    Node *temp = fileManager->temp.front;
    while (temp != NULL) {
        Item *itemToPaste = (Item *)temp->data;
        Tree foundTree = searchTree(fileManager->root, *itemToPaste);
        if (foundTree == NULL) {
            printf("File tidak ditemukan\n");
            return;
        }
        char *path = TextFormat("%s/%s", fileManager->currentPath, itemToPaste->name);
        if (isCopy) {
            if (foundTree->item.type == ITEM_FOLDER) {
                if (DirectoryExists(path)) {
                    path = _createDuplicatedFolderName(path, "(1)");
                }
                if (MakeDirectory(path) != 0) {
                    printf("Gagal membuat folder\n");
                    return;
                }
            } else if (foundTree->item.type == ITEM_FILE) {
                if (FileExists(path)) {
                    path = _createDuplicatedFileName(path, "(1)");
                }
                FILE *newFile = fopen(path, "w");
                if (newFile == NULL) {
                    printf("Gagal membuat file %s\n", itemToPaste->name);
                    return;
                }
                fclose(newFile);
            }
        } else {
            // cut
            // duplikat ke tujuan
            if (foundTree->item.type == ITEM_FOLDER) {
                if (DirectoryExists(path)) {
                    path = _createDuplicatedFolderName(path, "(1)");
                }
                if (MakeDirectory(path) != 0) {
                    printf("Gagal membuat folder\n");
                    return;
                }
            } else if (foundTree->item.type == ITEM_FILE) {
                if (FileExists(path)) {
                    path = _createDuplicatedFileName(path, "(1)");
                }
                FILE *newFile = fopen(path, "w");
                if (newFile == NULL) {
                    printf("Gagal membuat file %s\n", itemToPaste->name);
                    return;
                }
                fclose(newFile);
            }

            // hapus item di origin path
            char *originPath = TextFormat("%s/%s", foundTree->item.path, foundTree->item.name);
            if (isDirectory(originPath)) {
                if (RemoveItemsRecurse(originPath) != 0) {
                    printf("Gagal menghapus folder %s\n", foundTree->item.name);
                    return;
                }
            } else {
                if (remove(originPath) != 0) {
                    printf("Gagal menghapus file %s\n", foundTree->item.name);
                    return;
                }
            }
        }
    }
}

char *_getNameFromPath(char *path) {
    char *name = strrchr(path, '/'); // dapatkan string yang dimulai dari karakter slash (/) terakhir
    if (name != NULL) {
        return name + 1; // skip karakter slash (/) terakhir
    }
    return path; // kembalikan pathnya kalau gak ada slash (/) (ini berarti sudah nama file)
};

void selectFile(FileManager *fileManager, Item item) {
    if (fileManager->selectedItem.head == NULL) {
        fileManager->selectedItem.head = (Node *)malloc(sizeof(Node));
        fileManager->selectedItem.head->data = malloc(sizeof(Item));
        *(Item *)fileManager->selectedItem.head->data = item;
        fileManager->selectedItem.head->next = NULL;
    } else {
        Node *temp = fileManager->selectedItem.head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = (Node *)malloc(sizeof(Node));
        temp->next->data = malloc(sizeof(Item));
        *(Item *)temp->next->data = item;
        temp->next->next = NULL;
    }
}

void clearSelectedFile(FileManager *fileManager) {
    Node *temp = fileManager->selectedItem.head;
    while (temp != NULL) {
        Node *next = temp->next;
        free(temp->data);
        free(temp);
        temp = next;
    }
    fileManager->selectedItem.head = NULL;
}

void deselectFile(FileManager *fileManager, Item item) {
    Node *temp = fileManager->selectedItem.head;
    Node *prev = NULL;

    while (temp != NULL) {
        Item data = *(Item *)temp->data;
        if (data.path == item.path && data.name == item.name) {
            if (prev == NULL) {
                fileManager->selectedItem.head = temp->next;
            } else {
                prev->next = temp->next;
            }
            free(temp->data);
            free(temp);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
}

bool isDirectory(char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

char *_createDuplicatedFolderName(char *filePath, char *suffix) {
    char *newPath = TextFormat("%s%s", filePath, suffix);
    if (DirectoryExists(newPath)) {
        newPath = _createDuplicatedFolderName(newPath, suffix);
    }
    return newPath;
}

char *_createDuplicatedFileName(char *filePath, char *suffix) {
    size_t len;
    char *extention = strrchr(filePath, '.');
    if (extention) {
        len = extention - filePath;
    } else {
        len = strlen(filePath);
    }
    char *nameOnly = (char *)malloc(len + 1);
    strncpy(nameOnly, filePath, len);
    nameOnly[len] = '\0';

    char *newPath = TextFormat("%s%s%s", nameOnly, suffix, extention);
    if (FileExists(newPath)) {
        newPath = _createDuplicatedFileName(newPath, suffix);
    }
    return newPath;
}

void windowsOpenWith(char *path) {
    printf("%s\n", path);

    char *command = "cmd /c start \"\"";
    int length = strlen(command) + strlen(path) + 5;

    char *executeableCommand = malloc(length);

    printf("%d\n", length);

    snprintf(executeableCommand, length, "%s \"%s\" /OPENAS", command, path);

    printf("%s\n", executeableCommand);

    system(executeableCommand);

    free(executeableCommand);
}

Tree getCurrentRoot(FileManager *fileManager) {
    if (fileManager == NULL || fileManager->treeCursor == NULL) {
        return NULL;
    }

    Tree currentRoot = fileManager->treeCursor;
    while (currentRoot->parent != NULL) {
        currentRoot = currentRoot->parent;
    }
    return currentRoot;
}

char *getCurrentPath(Tree tree) {
    char *path = strdup("");
    if (!path)
        return NULL;

    while (tree != NULL) {
        char *name = tree->item.name;
        size_t newLen = strlen(name) + strlen(path) + 2;

        char *newPath = malloc(newLen);
        if (!newPath) {
            free(path);
            return NULL;
        }

        if (tree->parent == NULL) {
            snprintf(newPath, newLen, "%s%s", name, path);
        } else {
            snprintf(newPath, newLen, "/%s%s", name, path);
        }
        free(path);
        path = newPath;
        tree = tree->parent;
    }

    return path;
}

void goTo(FileManager *fileManager, Tree tree) {
    if (!fileManager || !tree)
        return;

    fileManager->treeCursor = tree;

    char *newPath = getCurrentPath(tree);
    if (newPath) {
        // free(fileManager->currentPath);
        fileManager->currentPath = newPath;
    }

    printf("%s\n", newPath);
}

void goBack(FileManager *fileManager) {
    if (!fileManager || !fileManager->treeCursor)
        return;

    Tree parent = fileManager->treeCursor->parent;
    if (parent)
        goTo(fileManager, parent);
}

void sort_children(Tree *parent) {
    if (!parent || !(*parent) || !(*parent)->first_son || !(*parent)->first_son->next_brother)
        return;

    Tree head = (*parent)->first_son;
    Tree sorted = NULL;

    while (head) {
        Tree current = head;
        head = head->next_brother;

        if (!sorted || current->item.type < sorted->item.type) {
            current->next_brother = sorted;
            sorted = current;
        } else {
            Tree temp = sorted;
            while (temp->next_brother && current->item.type >= temp->next_brother->item.type) {
                temp = temp->next_brother;
            }
            current->next_brother = temp->next_brother;
            temp->next_brother = current;
        }
    }

    (*parent)->first_son = sorted;
}
