#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "file_manager.h"
#include "item.h"
#include "nbtree.h"
#include "operation.h"
#include "queue.h"
#include "raylib.h"
#include "stack.h"
#include "utils.h"
#include "win_utils.h"
#include <string.h>

#include <time.h>
#define ROOT ".dir/root"
#define TRASH ".dir/trash"

bool isCopy = 0;

/* IS:                        FS:
  Tree root          = ?;     ==> NULL;
  Tree rootTrash     = ?;     ==> NULL;
  Stack actionStack  = ?;     ==> NULL;
  Queue selectedItem = ?;     ==> NULL;
  Queue currentPath  = ?;     ==> NULL;
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

/* IS:                          FS:
  Tree root          = NULL;    ==> loadTree(*filemanager, "./root/dir/");
  Tree rootTrash     = NULL;    ==> loadTree(*filemanager, "./root/trash/");
  Stack actionStack  = NULL;    ==> NULL;
  Queue selectedItem = NULL;    ==> NULL;
  Queue currentPath  = NULL;    ==> enqueue("./root/");
*/
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

/*
 * IS:
 * FS:
================================================================================*/
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

// == FILE OPERATION

/*
 * IS:
 * FS:
================================================================================*/
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

/*
 * IS:
 * FS:
================================================================================*/
void deleteFile(FileManager *fileManager) {
    if (fileManager->selectedItem.head == NULL) {
        printf("Tidak ada file yang dipilih untuk dihapus\n");
        return;
    }

    Node *temp = fileManager->selectedItem.head;
    while (temp != NULL) {
        Item *itemToDelete = (Item *)temp->data;
        Tree foundTree = searchTree(fileManager->root, *itemToDelete);
        if (foundTree == NULL) {
            printf("File %s tidak ditemukan\n", itemToDelete->name);
            temp = temp->next;
            continue;
        }

        // Pindah ke trash, bukan hapus permanen
        _moveToTrash(foundTree);
        temp = temp->next;
    }

    clearSelectedFile(fileManager);
    printf("File berhasil dipindah ke trash\n");
}

/* Procedur untuk Rename/Update item name
 *  IS: Nama item belum berubah
 *  FS: Nama item berubah
 * note: file path adalah full path, bukan path direktori
================================================================================
*/
void renameFile(FileManager *fileManager, char *filePath, char *newName) {
    Item item;
    Tree foundTree;
    char *newPath;
    // Cari item
    item = createItem(_getNameFromPath(filePath), filePath, 0, 0, 0, 0, 0);
    foundTree = searchTree(fileManager->treeCursor, item);
    if (foundTree == NULL) {
        printf("File tidak ditemukan\n");
        return;
    }
    newPath = TextFormat("%s/%s", fileManager->currentPath, newName);
    // rename file
    if (foundTree->item.type == ITEM_FILE) {
        if (DirectoryExists(newPath)) {
            newPath = _createDuplicatedFileName(newPath, "(1)");
        }
    } else if (foundTree->item.type == ITEM_FILE) {
        if (FileExists(newPath)) {
            newPath = _createDuplicatedFolderName(newPath, "(1)");
        }
    }
    rename(filePath, newPath);

    // update item
    foundTree->item.name = strdup(newName);
    foundTree->item.path = strdup(newPath);

    printf("File berhasil diubah namanya menjadi %s\n", newName);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void recoverFile(FileManager *fileManager) {
    if (fileManager->selectedItem.head == NULL) {
        printf("Tidak ada file yang dipilih untuk di-recover\n");
        return;
    }

    Node *temp = fileManager->selectedItem.head;
    while (temp != NULL) {
        Item *itemToRecover = (Item *)temp->data;

        // Cek apakah item ada di trash
        char *trashPath = TextFormat(".dir/trash/%s", itemToRecover->name);
        if (!FileExists(trashPath) && !DirectoryExists(trashPath)) {
            printf("File %s tidak ditemukan di trash\n", itemToRecover->name);
            temp = temp->next;
            continue;
        }

        // Kembalikan ke lokasi asli atau current path
        char *recoverPath = TextFormat("%s/%s", fileManager->currentPath, itemToRecover->name);

        // Kalau udah ada, buat nama unik
        if (FileExists(recoverPath) || DirectoryExists(recoverPath)) {
            if (itemToRecover->type == ITEM_FOLDER) {
                recoverPath = _createDuplicatedFolderName(recoverPath, "(recovered)");
            } else {
                recoverPath = _createDuplicatedFileName(recoverPath, "(recovered)");
            }
        }

        // Move dari trash ke lokasi recovery
        if (rename(trashPath, recoverPath) == 0) {
            printf("File %s berhasil di-recover\n", itemToRecover->name);
        } else {
            printf("Gagal me-recover file %s\n", itemToRecover->name);
        }

        temp = temp->next;
    }

    clearSelectedFile(fileManager);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
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

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
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

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
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

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void pasteFile(FileManager *fileManager) {
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
        char *originPath = TextFormat("%s/%s", foundTree->item.path, foundTree->item.name);

        // COPY/CUT sama-sama butuh copy content dulu
        if (foundTree->item.type == ITEM_FOLDER) {
            if (DirectoryExists(path)) {
                path = _createDuplicatedFolderName(path, "(1)");
            }
            if (MakeDirectory(path) != 0) {
                printf("Gagal membuat folder\n");
                return;
            }
            _copyFolderRecursive(originPath, path);
        } else if (foundTree->item.type == ITEM_FILE) {
            if (FileExists(path)) {
                path = _createDuplicatedFileName(path, "(1)");
            }
            _copyFileContent(originPath, path);
        }

        // Kalau cut, gunakan fungsi delete helper
        if (!isCopy) {
            _deleteSingleItem(originPath, foundTree->item.type, foundTree->item.name);
        }

        temp = temp->next;
    }
    printf("Paste berhasil!\n");
}

// === SELECTING ITEM

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
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

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
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

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
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

// === UNDO/REDO

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void undo(FileManager *fileManager) {
    Operation *operationToUndo;
    Tree foundTree;
    if (fileManager->undo == NULL) {
        printf("No actions to undo.\n");
        return;
    }
    operationToUndo = alloc(Operation);
    operationToUndo = (Operation *)pop(&(fileManager->undo));
    push(&(fileManager->redo), operationToUndo);
    switch (operationToUndo->type) {
    case ACTION_CREATE:
        // Hapus item yang baru dibuat
        foundTree = searchTree(fileManager->root, createItem(_getNameFromPath(operationToUndo->from), operationToUndo->from, 0, ITEM_FILE, 0, 0, 0));
        if (foundTree != NULL) {
            _deleteSingleItem(foundTree->item.path, foundTree->item.type, foundTree->item.name);
            printf("Undo create: %s\n", operationToUndo->from);
        }
        break;
    case ACTION_DELETE:
        // Kembalikan item yang dihapus
        foundTree = searchTree(fileManager->rootTrash, createItem(_getNameFromPath(operationToUndo->from), operationToUndo->from, 0, ITEM_FILE, 0, 0, 0));
        if (foundTree != NULL) {
        }
        break;
    case ACTION_UPDATE:
        // Kembalikan nama item yang diubah
        foundTree = searchTree(fileManager->root, createItem(_getNameFromPath(operationToUndo->from), operationToUndo->from, 0, ITEM_FILE, 0, 0, 0));
        if (foundTree != NULL) {
            renameFile(fileManager, foundTree->item.path, operationToUndo->to);
            printf("Undo update: %s to %s\n", operationToUndo->from, operationToUndo->to);
        }
        break;
    case ACTION_RECOVER:
        // Hapus item yang sudah di-recover
        foundTree = searchTree(fileManager->root, createItem(_getNameFromPath(operationToUndo->from), operationToUndo->from, 0, ITEM_FILE, 0, 0, 0));
        if (foundTree != NULL) {
            _moveToTrash(foundTree);
            printf("Undo recover: %s\n", operationToUndo->from);
        }
        break;
    case ACTION_PASTE:
        // Hapus item yang sudah di-paste

        break;

    default:
        break;
    }
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void redo(FileManager *fileManager) {
}

// ================================================================================
// . . . HELPER FUNC / PROC . . .
// ================================================================================

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
char *_getNameFromPath(char *path) {
    char *name = strrchr(path, '/'); // dapatkan string yang dimulai dari karakter slash (/) terakhir
    if (name != NULL) {
        return name + 1; // skip karakter slash (/) terakhir
    }
    return path; // kembalikan pathnya kalau gak ada slash (/) (ini berarti sudah nama file)
};

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void _moveToTrash(Tree itemTree) {
    char *trashDir = ".dir/trash";
    if (!DirectoryExists(trashDir)) {
        MakeDirectory(trashDir);
    }

    char *srcPath = TextFormat("%s/%s", itemTree->item.path, itemTree->item.name);
    char *trashPath = TextFormat("%s/%s", trashDir, itemTree->item.name);

    // Kalau sudah ada di trash, buat nama unik
    if (FileExists(trashPath) || DirectoryExists(trashPath)) {
        if (itemTree->item.type == ITEM_FOLDER) {
            trashPath = _createDuplicatedFolderName(trashPath, "(1)");
        } else {
            trashPath = _createDuplicatedFileName(trashPath, "(1)");
        }
    }

    // Move ke trash
    rename(srcPath, trashPath);
}

/*  Prosedur untuk hapus permanen (untuk undo operations)
 *  IS:
 *  FS:
================================================================================*/
void _deletePermanently(char *fullPath, ItemType type, char *name) {
    if (type == ITEM_FOLDER) {
        if (RemoveItemsRecurse(fullPath) != 0) {
            printf("Gagal menghapus folder %s\n", name);
        }
    } else {
        if (remove(fullPath) != 0) {
            printf("Gagal menghapus file %s\n", name);
        }
    }
}

/*  Prosedur Fungsi helper untuk cut operation (hapus permanen)
 *  IS:
 *  FS:
================================================================================*/
void _deleteSingleItem(char *fullPath, ItemType type, char *name) {
    // Cut operation = hapus permanen (tidak ke trash)
    _deletePermanently(fullPath, type, name);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void _copyFileContent(char *srcPath, char *destPath) {
    FILE *src = fopen(srcPath, "rb");
    FILE *dest = fopen(destPath, "wb");

    if (!src || !dest) {
        if (src)
            fclose(src);
        if (dest)
            fclose(dest);
        return;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dest);
    }

    fclose(src);
    fclose(dest);
}

/* FungsProseduri rekursif untuk copy folder
 *  IS:
 *  FS:
================================================================================*/
void _copyFolderRecursive(char *srcPath, char *destPath) {
    DIR *dp = opendir(srcPath);
    if (!dp)
        return;

    struct dirent *ep;
    struct stat statbuf;

    while ((ep = readdir(dp)) != NULL) {
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;

        char *srcItem = TextFormat("%s/%s", srcPath, ep->d_name);
        char *destItem = TextFormat("%s/%s", destPath, ep->d_name);

        if (stat(srcItem, &statbuf) == -1)
            continue;

        if (S_ISDIR(statbuf.st_mode)) {
            // Buat folder dan copy rekursif
            MakeDirectory(destItem);
            _copyFolderRecursive(srcItem, destItem);
        } else {
            // Copy file content
            _copyFileContent(srcItem, destItem);
        }
    }
    closedir(dp);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
bool isDirectory(char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
char *_createDuplicatedFolderName(char *filePath, char *suffix) {
    char *newPath = TextFormat("%s%s", filePath, suffix);
    if (DirectoryExists(newPath)) {
        newPath = _createDuplicatedFolderName(newPath, suffix);
    }
    return newPath;
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
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

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
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

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
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

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
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

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void goBack(FileManager *fileManager) {
    if (!fileManager || !fileManager->treeCursor)
        return;

    Tree parent = fileManager->treeCursor->parent;
    if (parent)
        goTo(fileManager, parent);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
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
