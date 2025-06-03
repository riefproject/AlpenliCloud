#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "component.h"
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

#include "ctx.h"
#include "gui/sidebar.h"

#include <time.h>
#define _DIR ".dir/"
#define ROOT ".dir/root"
#define TRASH ".dir/trash"
#define TRASH_DUMP ".dir/trash_dump.txt"

bool isCopy = 0;

/* IS:                        FS:
  Tree root          = ?;     ==> NULL;
  Tree rootTrash     = ?;     ==> NULL;
  Stack actionStack  = ?;     ==> NULL;
  Queue selectedItem = ?;     ==> NULL;
  Queue currentPath  = ?;     ==> NULL;
*/
void createFileManager(FileManager* fileManager) {
    create_tree(&(fileManager->root));
    create_list(&(fileManager->trash));
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
void initFileManager(FileManager* fileManager) {
    Item rootItem;
    if (fileManager->root == NULL) {
        rootItem = createItem("root", ROOT, 0, ITEM_FOLDER, 0, 0, 0);
        fileManager->root = create_node_tree(rootItem);
        fileManager->currentPath = "root";

        fileManager->treeCursor = fileManager->root;

        loadTree(fileManager->treeCursor, ROOT);

        loadTrashFromFile(&(fileManager->trash));

        printf("\n\n");
    }
}

/*
 * IS:
 * FS:
 * Author
================================================================================*/
void loadTree(Tree tree, char* path) {
    DIR* dp;
    struct dirent* ep;
    struct stat statbuf;

    dp = opendir(path);
    if (dp == NULL) {
        perror("Tidak dapat membuka direktori");
        return;
    }

    while ((ep = readdir(dp)) != NULL) {
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;

        char* fullPath = malloc(strlen(path) + strlen(ep->d_name) + 2);
        sprintf(fullPath, "%s/%s", path, ep->d_name);

        if (stat(fullPath, &statbuf) == -1) {
            perror("Gagal mendapatkan info file");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            Item data = createItem(ep->d_name, fullPath, statbuf.st_size, ITEM_FOLDER, statbuf.st_ctime, statbuf.st_mtime, 0);
            Tree newTree = insert_node(tree, data);
            loadTree(newTree, fullPath);
        }
        else if (S_ISREG(statbuf.st_mode)) {
            Item data = createItem(ep->d_name, fullPath, statbuf.st_size, ITEM_FILE, statbuf.st_ctime, statbuf.st_mtime, 0);
            insert_node(tree, data);
        }
        else {
            printf("[LOG]   (Tipe lain) Ditemukan: %s\n", fullPath);
        }

        free(fullPath);
    }

    closedir(dp);
    return;
}

/*
 * IS  : File `TRASH_DUMP` mungkin ada atau belum ada.
 * FS  : Data di dalam `TRASH_DUMP` dibaca dan diisi ke dalam antrian `fileManager->trash`.
 * Author: Farras Fadhil Syafiq
 ================================================================================*/
void loadTrashFromFile(LinkedList* trash) {
    FILE* trashFile = fopen(TRASH_DUMP, "r");
    if (trashFile == NULL) {
        perror("Gagal membuka file trash");
        return;
    }

    char* line = NULL;
    size_t len = 0;
    size_t capacity = 0;
    int ch;

    while ((ch = fgetc(trashFile)) != EOF) {
        // Perluas buffer jika perlu
        if (len + 1 >= capacity) {
            capacity = (capacity == 0) ? 128 : capacity * 2;
            char* newLine = realloc(line, capacity);
            if (!newLine) {
                free(line);
                fclose(trashFile);
                fprintf(stderr, "Gagal mengalokasikan memori untuk line\n");
                return;
            }
            line = newLine;
        }

        if (ch == '\n') {
            line[len] = '\0'; // Akhiri string

            // Proses line
            char* name = strtok(line, ",");
            char* originalPath = strtok(NULL, ",");
            char* deletedTimeStr = strtok(NULL, ",");

            if (name && originalPath && deletedTimeStr) {
                TrashItem* trashItem = alloc(TrashItem);
                if (!trashItem) {
                    len = 0; // reset untuk line selanjutnya
                    continue;
                }

                trashItem->item.name = strdup(name);
                trashItem->originalPath = strdup(originalPath);
                trashItem->deletedTime = atol(deletedTimeStr);
                trashItem->trashPath = strdup(TextFormat("%s/%s", TRASH_DUMP, name));

                insert_first(trash, trashItem);
            }

            len = 0; // Reset untuk line selanjutnya
        }
        else {
            line[len++] = ch;
        }
    }

    // Tangani kasus baris terakhir tanpa newline
    if (len > 0) {
        line[len] = '\0';
        char* name = strtok(line, ",");
        char* originalPath = strtok(NULL, ",");
        char* deletedTimeStr = strtok(NULL, ",");

        if (name && originalPath && deletedTimeStr) {
            TrashItem* trashItem = alloc(TrashItem);
            if (trashItem) {
                trashItem->item.name = strdup(name);
                trashItem->originalPath = strdup(originalPath);
                trashItem->deletedTime = atol(deletedTimeStr);
                trashItem->trashPath = strdup(TextFormat("%s/%s", TRASH_DUMP, name));

                insert_first(trash, trashItem);
            }
        }
    }

    free(line);
    fclose(trashFile);
}

/*
 * IS  : Antrian `fileManager->trash` sudah terisi.
 * FS  : Menyimpan isi antrian trash ke dalam file `TRASH_DUMP`.
 * Author: Farras Fadhil Syafiq
 ================================================================================*/
void saveTrashToFile(FileManager* fileManager) {
    FILE* trashFile = fopen(TRASH_DUMP, "w");
    if (trashFile == NULL) {
        perror("Gagal membuka file trash untuk menulis");
        return;
    }

    Node* current = fileManager->trash.head;
    while (current != NULL) {
        TrashItem* trashItem = (TrashItem*)current->data;
        if (trashItem && trashItem->item.name && trashItem->originalPath) {
            fprintf(trashFile, "%s,%s,%ld\n",
                trashItem->item.name,
                trashItem->originalPath,
                trashItem->deletedTime);
        }
        current = current->next;
    }

    fclose(trashFile);
}

/*
 * IS:
 * FS:
 * Author:
================================================================================*/
void printTrash(LinkedList trash) {
    Node* current = trash.head;
    if (current == NULL) {
        printf("[LOG] Trash is empty\n");
        return;
    }

    while (current != NULL) {
        TrashItem* trashItem = (TrashItem*)current->data;
        if (trashItem != NULL) {
            printf("[LOG] Trash Item: %s, Original Path: %s, Deleted Time: %ld, Trash Path: %s\n",
                trashItem->item.name,
                trashItem->originalPath,
                trashItem->deletedTime,
                trashItem->trashPath);
        }
        current = current->next;
    }
}

/*
 * IS:
 * FS:
 * Author:
================================================================================*/
void destroyTree(Tree* tree) {
    if (*tree == NULL)
        return;

    destroyTree(&(*tree)->first_son);
    destroyTree(&(*tree)->next_brother);

    free((*tree)->item.name);
    free((*tree)->item.path);
    free(*tree);
    *tree = NULL;
}

/*
 * IS:
 * FS:
 * Author:
================================================================================*/
void refreshFileManager(FileManager* fileManager) {
    if (fileManager != NULL && fileManager->treeCursor != NULL) {
        printf("\n\n");
        printf("==========================================================\n");
        printf("[LOG] Refreshing FileManager...\n");
        printf("[LOG] Tree Cursor Path: %s\n", fileManager->treeCursor->item.name);
        printf("[LOG] Tree Cursor Full Path: %s\n", fileManager->treeCursor->item.path);
        printf("[LOG] Tree Cursor Type: %s\n", fileManager->treeCursor->item.type == ITEM_FOLDER ? "Folder" : "File");
        printf("==========================================================\n\n");

        // Hapus semua anak dari direktori saat ini, bukan seluruh node
        destroyTree(&fileManager->treeCursor->first_son);
        fileManager->treeCursor->first_son = NULL;

        // Muat ulang isi dari direktori tersebut
        loadTree(fileManager->treeCursor, fileManager->treeCursor->item.path);

        printf("==========================================================\n");
        printTree(fileManager->treeCursor, 0);
        printf("==========================================================\n\n");

        printf("[LOG] Directory refreshed successfully\n");

        printf("[LOG] Resfreshing sidebar...\n");
        SidebarState* stateList = NULL;
        collectSidebarState(fileManager->ctx->sidebar->sidebarRoot, &stateList);
        destroySidebarItem(&fileManager->ctx->sidebar->sidebarRoot);
        fileManager->ctx->sidebar->sidebarRoot = NULL;
        fileManager->ctx->sidebar->sidebarRoot = createSidebarItemWithState(fileManager->root, stateList);
        destroySidebarState(stateList);

        printf("[LOG] Sidebar refreshed successfully\n");
    }
}

// == FILE OPERATION

/*
 * IS:
 * FS:
================================================================================*/
void createFile(FileManager* fileManager, ItemType type, char* dirPath, char* name, bool isOperation) {
    Item newItem;
    char* path;
    char* currentFullPath;
    Tree currentNode;
    time_t createdTime;
    FILE* newFile;
    Operation* newOperation;

    if (strlen(name) + 1 >= 255) {
        printf("[LOG] nama file terlalu panjang, gagal membuat file\n");
        return;
    }

    currentFullPath = strdup(dirPath);
    currentNode = searchTree(fileManager->root, createItem(_getNameFromPath(currentFullPath), currentFullPath, 0, ITEM_FOLDER, 0, 0, 0));
    // printf("[LOG] \ncurrent path: %s\n", currentFullPath);
    // printf("[LOG] current name: %s\n", _getNameFromPath(currentFullPath));
    if (currentNode != NULL) {
        path = TextFormat("%s/%s", currentFullPath, name);
        createdTime = time(NULL);
        if (type == ITEM_FOLDER) {
            if (DirectoryExists(path)) {
                path = _createDuplicatedFolderName(path, "(1)");
            }
            if (MakeDirectory(path) != 0) {
                printf("[LOG] Gagal membuat folder\n");
                return;
            }
        }
        else if (type == ITEM_FILE) {
            if (FileExists(path)) {
                path = _createDuplicatedFileName(path, "(1)");
            }
            newFile = fopen(path, "w");
            if (newFile == NULL) {
                printf("[LOG] Gagal membuat file %s\n", name);
                return;
            }
            fclose(newFile);
        }

        if (isOperation) {
            newOperation = alloc(Operation);
            *newOperation = createOperation(path, NULL, ACTION_CREATE, false, NULL);
            push(&fileManager->undo, newOperation);
        }
        newItem = createItem(_getNameFromPath(path), path, 0, type, createdTime, createdTime, -1);
        insert_node(currentNode, newItem);

        refreshFileManager(fileManager);
    }
    else {
        printf("[LOG] Direktori parent tidak ditemukan : %s\n", dirPath);
    }
}

/*
 * IS:
 * FS:
================================================================================*/
void deleteFile(FileManager* fileManager, bool isOperation) {
    Operation* deleteOperation;
    if (fileManager->selectedItem.head == NULL) {
        printf("[LOG] Tidak ada file yang dipilih untuk dihapus\n");
        return;
    }

    Node* temp = fileManager->selectedItem.head;
    if (isOperation) {
        deleteOperation = alloc(Operation);
        *deleteOperation = createOperation(NULL, NULL, ACTION_DELETE, false, NULL);
        deleteOperation->itemTemp = alloc(Queue);
        create_queue(&(*(deleteOperation->itemTemp)));
    }
    while (temp != NULL) {
        Item* itemToDelete = (Item*)temp->data;
        Tree foundTree = searchTree(fileManager->root, *itemToDelete);
        if (foundTree == NULL) {
            printf("[LOG] File %s tidak ditemukan\n", itemToDelete->name);
            temp = temp->next;
            continue;
        }

        // printf("[LOG] File berhasil dipindah ke trash\n");
        // Tambahkan item ke queue dalam operasi
        if (isOperation) {
            TrashItem* trashItem = alloc(TrashItem);
            trashItem->item = foundTree->item;
            trashItem->originalPath = strdup(foundTree->item.path);
            trashItem->trashPath = strdup(TextFormat("%s/%s", TRASH, foundTree->item.name));
            enqueue(deleteOperation->itemTemp, trashItem);
            printf("[LOG] Operasi delete berhasil dibuat dengan origin path:%s\n", trashItem->originalPath);
        }

        _moveToTrash(fileManager, foundTree);
        temp = temp->next;
    }

    push(&fileManager->undo, deleteOperation);
    clearSelectedFile(fileManager);
    printf("[LOG] File berhasil dipindah ke trash\n");
}

/* Procedur untuk Rename/Update item name
 *  IS: Nama item belum berubah
 *  FS: Nama item berubah
 * note: file path adalah full path, bukan path direktori
================================================================================
*/
void renameFile(FileManager* fileManager, char* filePath, char* newName, bool isOperation) {
    Item item;
    Tree foundTree;
    char* newPath;
    Operation* operationToUndo;
    if (isOperation) {
        operationToUndo = alloc(Operation);
    }
    // Cari item
    item = createItem(_getNameFromPath(filePath), filePath, 0, 0, 0, 0, 0);
    foundTree = searchTree(fileManager->treeCursor, item);
    if (foundTree == NULL) {
        printf("[LOG] File tidak ditemukan\n");
        return;
    }
    newPath = TextFormat("%s/%s", _getDirectoryFromPath(filePath), newName);
    // rename file
    if (foundTree->item.type == ITEM_FILE) {
        if (DirectoryExists(newPath)) {
            newPath = _createDuplicatedFileName(newPath, "(1)");
        }
    }
    else if (foundTree->item.type == ITEM_FILE) {
        if (FileExists(newPath)) {
            newPath = _createDuplicatedFolderName(newPath, "(1)");
        }
    }
    if (isOperation) {
        // Simpan operasi untuk undo
        *operationToUndo = createOperation(filePath, newPath, ACTION_UPDATE, false, NULL);
        operationToUndo->isDir = (foundTree->item.type == ITEM_FOLDER);
        operationToUndo->itemTemp = NULL;
        push(&fileManager->undo, operationToUndo);
    }
    rename(filePath, newPath);

    // update item
    foundTree->item.name = strdup(newName);
    foundTree->item.path = strdup(newPath);

    printf("[LOG] File berhasil diubah namanya menjadi %s\n", newName);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void recoverFile(FileManager* fileManager) {
    if (fileManager->selectedItem.head == NULL) {
        printf("[LOG] Tidak ada file yang dipilih untuk di-recover\n");
        return;
    }

    Node* temp = fileManager->selectedItem.head;
    while (temp != NULL) {
        Item* itemToRecover = (Item*)temp->data;

        // Cari di LinkedList trash
        Node* trashNode = fileManager->trash.head;
        TrashItem* foundTrashItem = NULL;

        while (trashNode != NULL) {
            TrashItem* trashItem = (TrashItem*)trashNode->data;
            if (strcmp(trashItem->item.name, itemToRecover->name) == 0) {
                foundTrashItem = trashItem;
                break;
            }
            trashNode = trashNode->next;
        }

        if (foundTrashItem == NULL) {
            printf("[LOG] File %s tidak ditemukan di trash\n", itemToRecover->name);
            temp = temp->next;
            continue;
        }

        // Recover ke original path atau current path
        char* recoverPath = TextFormat("%s/%s", foundTrashItem->originalPath, foundTrashItem->item.name);

        // Handle nama duplikat
        if (FileExists(recoverPath) || DirectoryExists(recoverPath)) {
            if (foundTrashItem->item.type == ITEM_FOLDER) {
                recoverPath = _createDuplicatedFolderName(recoverPath, "(recovered)");
            }
            else {
                recoverPath = _createDuplicatedFileName(recoverPath, "(recovered)");
            }
        }

        // Move dari trash ke lokasi recovery
        if (rename(foundTrashItem->trashPath, recoverPath) == 0) {
            printf("[LOG] File %s berhasil di-recover\n", foundTrashItem->item.name);

            // Hapus dari LinkedList trash
            _removeFromTrash(fileManager, foundTrashItem->item.name);

            // Tambah kembali ke tree
            _addBackToTree(fileManager, foundTrashItem, recoverPath);
        }
        else {
            printf("[LOG] Gagal me-recover file %s\n", foundTrashItem->item.name);
        }

        temp = temp->next;
    }

    clearSelectedFile(fileManager);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
Item searchFile(FileManager* fileManager, char* path) {
    Item item = { 0 };
    Item itemToSearch;
    Tree foundTree;

    itemToSearch = createItem(_getNameFromPath(path), path, 0, ITEM_FILE, 0, 0, 0);

    foundTree = searchTree(fileManager->root, itemToSearch);

    if (foundTree == NULL) {
        printf("[LOG] File tidak ditemukan\n");
        return item;
    }
    item = foundTree->item;
    return item;
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void copyFile(FileManager* fileManager) {
    if (fileManager->copied.front)
        fileManager->copied.front = NULL;
    Node* temp = fileManager->selectedItem.head;
    while (temp != NULL) {
        Item* itemToCopy = (Item*)temp->data;
        enqueue(&(fileManager->copied), itemToCopy);
        temp = temp->next;
    }
    isCopy = true;
    if (fileManager->copied.front == NULL) {
        printf("[LOG] Gagal Menyalin File!\n");
        return;
    }
    fileManager->temp = fileManager->copied;
    printf("[LOG] File berhasil disalin ke clipboard\n");
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void cutFile(FileManager* fileManager) {
    if (fileManager->copied.front)
        fileManager->copied.front = NULL;
    Node* temp = fileManager->selectedItem.head;
    while (temp != NULL) {
        Item* itemToCopy = (Item*)temp->data;
        enqueue(&(fileManager->copied), itemToCopy);
        temp = temp->next;
    }
    isCopy = false;
    if (fileManager->copied.front == NULL) {
        printf("[LOG] Gagal Menyalin File!\n");
        return;
    }
    fileManager->temp = fileManager->copied;
    printf("[LOG] File berhasil disalin ke clipboard\n");
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void pasteFile(FileManager* fileManager) {
    if (fileManager->temp.front == NULL) {
        printf("[LOG] Clipboard kosong\n");
        return;
    }

    // Reset progress state
    resetProgressBarState();

    // Hitung total item untuk progress bar
    int totalItems = 0;
    Node* countNode = fileManager->temp.front;
    while (countNode != NULL) {
        totalItems++;
        countNode = countNode->next;
    }

    // Cek apakah perlu menampilkan progress bar
    bool showProgress = shouldShowProgressBar(totalItems);
    int currentProgress = 0;
    bool cancelled = false;

    printf("[LOG] Total items to paste: %d, Show progress: %s\n",
        totalItems, showProgress ? "true" : "false");

    Node* temp = fileManager->temp.front;
    while (temp != NULL && !cancelled) {
        Item* itemToPaste = (Item*)temp->data;

        // Update progress bar dan cek cancel
        if (showProgress) {
            showPasteProgressBar(currentProgress, totalItems, itemToPaste->name);

            // Cek apakah user menekan cancel
            if (shouldCancelPaste()) {
                printf("[LOG] Paste operation cancelled by user\n");
                cancelled = true;
                break;
            }
        }

        // Path untuk file/folder baru di lokasi tujuan
        char* destinationFullPath = TextFormat("%s%s", _DIR, fileManager->currentPath);
        printf("\n%s\n", fileManager->currentPath);
        char* newPath = TextFormat("%s/%s", destinationFullPath, itemToPaste->name);
        char* originPath = itemToPaste->path;

        printf("[LOG] Mencoba paste: %s -> %s\n", originPath, newPath);
        printf("[LOG] Is Copy: %s\n", isCopy ? "true" : "false");
        printf("[LOG] Item type: %d (ITEM_FOLDER=%d, ITEM_FILE=%d)\n", itemToPaste->type, ITEM_FOLDER, ITEM_FILE);

        // COPY operation
        if (isCopy) {
            Tree foundTree = searchTree(fileManager->root, *itemToPaste);
            if (foundTree == NULL) {
                printf("[LOG] File tidak ditemukan untuk copy: %s\n", itemToPaste->name);
                temp = temp->next;
                currentProgress++;
                continue;
            }
            originPath = foundTree->item.path;
            printf("[LOG] Origin path updated to: %s\n", originPath);
        }

        // Cek apakah source file/folder masih ada
        if (!FileExists(originPath) && !DirectoryExists(originPath)) {
            printf("[LOG] Source tidak ditemukan: %s\n", originPath);
            temp = temp->next;
            currentProgress++;
            continue;
        }

        printf("[LOG] Source exists, proceeding with copy/cut\n");

        // COPY/CUT sama-sama butuh copy content dulu
        if (itemToPaste->type == ITEM_FOLDER) {
            printf("[LOG] Processing folder: %s\n", itemToPaste->name);

            if (DirectoryExists(newPath)) {
                printf("[LOG] Destination exists, creating duplicate name\n");
                newPath = _createDuplicatedFolderName(newPath, "(1)");
                printf("[LOG] New path after duplicate check: %s\n", newPath);
            }

            // Untuk CUT, pindah langsung. Untuk COPY, buat folder baru lalu copy
            if (!isCopy) {
                printf("[LOG] CUT operation - moving folder\n");
                if (rename(originPath, newPath) != 0) {
                    printf("[LOG] Gagal memindahkan folder %s\n", itemToPaste->name);
                    temp = temp->next;
                    currentProgress++;
                    continue;
                }
            }
            else {
                printf("[LOG] COPY operation - copying folder\n");
                printf("[LOG] Membuat folder destination: %s\n", newPath);
                if (MakeDirectory(newPath) != 0) {
                    printf("[LOG] Gagal membuat folder\n");
                    temp = temp->next;
                    currentProgress++;
                    continue;
                }
                printf("[LOG] Folder destination created successfully\n");
                printf("[LOG] Mulai copy recursive dari %s ke %s\n", originPath, newPath);
                _copyFolderRecursive(originPath, newPath);
                printf("[LOG] Selesai copy recursive\n");
            }

            // Tambahkan ke tree struktur
            Tree currentNode = fileManager->treeCursor;
            if (currentNode != NULL) {
                Item newItem = createItem(
                    _getNameFromPath(newPath),
                    newPath,
                    itemToPaste->size,
                    ITEM_FOLDER,
                    itemToPaste->created_at,
                    time(NULL),
                    0);
                insert_node(currentNode, newItem);
                printf("[LOG] Folder %s berhasil ditambahkan ke tree\n", newItem.name);
            }
        }
        else if (itemToPaste->type == ITEM_FILE) {
            printf("[LOG] Processing file: %s\n", itemToPaste->name);

            if (FileExists(newPath)) {
                newPath = _createDuplicatedFileName(newPath, "(1)");
            }

            if (!isCopy) {
                if (rename(originPath, newPath) != 0) {
                    printf("[LOG] Gagal memindahkan file %s\n", itemToPaste->name);
                    temp = temp->next;
                    currentProgress++;
                    continue;
                }
            }
            else {
                _copyFileContent(originPath, newPath);
            }

            // Tambahkan item baru ke tree di lokasi tujuan
            Tree currentNode = fileManager->treeCursor;
            if (currentNode != NULL) {
                Item newItem = createItem(
                    _getNameFromPath(newPath),
                    newPath,
                    itemToPaste->size,
                    itemToPaste->type,
                    itemToPaste->created_at,
                    time(NULL),
                    0);
                insert_node(currentNode, newItem);
                printf("[LOG] Item %s berhasil ditambahkan ke tree\n", newItem.name);
            }
        }

        // Kalau cut, hapus dari tree
        if (!isCopy) {
            Tree foundTree = searchTree(fileManager->root, *itemToPaste);
            if (foundTree != NULL) {
                remove_node(&(fileManager->root), foundTree);
                printf("[LOG] Item %s berhasil dihapus dari tree asal\n", itemToPaste->name);
            }
        }

        temp = temp->next;
        currentProgress++;
    }

    // Reset progress state setelah selesai
    resetProgressBarState();

    // Clear clipboard setelah cut operation (hanya jika tidak di-cancel)
    if (!isCopy && !cancelled) {
        fileManager->temp.front = NULL;
    }

    if (cancelled) {
        printf("[LOG] Paste operation was cancelled\n");
    }
    else {
        printf("[LOG] Paste berhasil!\n");
    }

    refreshFileManager(fileManager);
}

// Update fungsi _reconstructTreeStructure untuk handle kasus cut
void _reconstructTreeStructure(FileManager* fileManager, Tree sourceTree, char* newBasePath, char* destinationPath) {
    // Fix: Use treeCursor directly instead of searching
    Tree parentNode = fileManager->treeCursor;

    if (parentNode == NULL) {
        printf("[LOG] Parent node tidak ditemukan\n");
        return;
    }

    // Buat node baru untuk folder utama
    Item newFolderItem = createItem(
        _getNameFromPath(newBasePath),
        newBasePath,
        0, // size - akan diupdate oleh loadTree
        ITEM_FOLDER,
        time(NULL), // created_at
        time(NULL), // modified_at
        0);

    Tree newFolderNode = insert_node(parentNode, newFolderItem);

    // Rekursif untuk semua child
    // _loadTreeFromPath(newFolderNode, newBasePath);
    loadTree(newFolderNode, newBasePath);
}

// Fungsi helper baru untuk load tree dari path yang sudah ada di filesystem
void _loadTreeFromPath(Tree parentNode, char* basePath) {
    DIR* dp;
    struct dirent* ep;
    struct stat statbuf;

    dp = opendir(basePath);
    if (dp == NULL) {
        return;
    }

    while ((ep = readdir(dp)) != NULL) {
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;

        char* fullPath = TextFormat("%s/%s", basePath, ep->d_name);

        if (stat(fullPath, &statbuf) == -1) {
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            Item folderItem = createItem(ep->d_name, fullPath, statbuf.st_size, ITEM_FOLDER, statbuf.st_ctime, statbuf.st_mtime, 0);
            Tree newFolderNode = insert_node(parentNode, folderItem);
            // Rekursif untuk subfolder
            _loadTreeFromPath(newFolderNode, fullPath);
        }
        else if (S_ISREG(statbuf.st_mode)) {
            Item fileItem = createItem(ep->d_name, fullPath, statbuf.st_size, ITEM_FILE, statbuf.st_ctime, statbuf.st_mtime, 0);
            insert_node(parentNode, fileItem);
        }
    }

    closedir(dp);
}

// Hapus fungsi _reconstructChildNodes karena kita pakai approach yang lebih sederhana
// === SELECTING ITEM

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void selectFile(FileManager* fileManager, Item* item) {
    Node* temp = fileManager->selectedItem.head;
    while (temp != NULL) {
        Item* existingItem = (Item*)temp->data;
        if (strcmp(existingItem->path, item->path) == 0 &&
            strcmp(existingItem->name, item->name) == 0) {
            // Item sudah ada, tidak perlu ditambahkan lagi
            return;
        }
        temp = temp->next;
    }

    // Item belum ada, tambahkan ke list
    item->selected = true; // Tandai item sebagai terpilih
    if (fileManager->selectedItem.head == NULL) {
        fileManager->selectedItem.head = (Node*)malloc(sizeof(Node));
        fileManager->selectedItem.head->data = malloc(sizeof(Item));
        fileManager->selectedItem.head->data = item;
        fileManager->selectedItem.head->next = NULL;
    }
    else {
        temp = fileManager->selectedItem.head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = (Node*)malloc(sizeof(Node));
        temp->next->data = malloc(sizeof(Item));
        temp->next->data = item;
        temp->next->next = NULL;
    }
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void clearSelectedFile(FileManager* fileManager) {
    Node* temp = fileManager->selectedItem.head;
    while (temp != NULL) {
        deselectFile(fileManager, (Item*)temp->data);
        temp = temp->next;
    }
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void deselectFile(FileManager* fileManager, Item* item) {
    Node* temp = fileManager->selectedItem.head;
    Node* prev = NULL;

    while (temp != NULL) {
        Item* data = (Item*)temp->data;
        if (data->path == item->path && data->name == item->name) {
            item->selected = false;
            if (prev == NULL) {
                fileManager->selectedItem.head = temp->next;
            }
            else {
                prev->next = temp->next;
            }
            return;
        }
        prev = temp;
        temp = temp->next;
    }
    destroy_list(&fileManager->selectedItem);
    create_list(&fileManager->selectedItem);
}

void selectAll(FileManager* fileManager) {
    if (fileManager->treeCursor == NULL) {
        printf("[LOG] Tidak ada direktori yang dipilih\n");
        return;
    }

    Tree currentNode = fileManager->treeCursor->first_son;
    while (currentNode != NULL) {
        selectFile(fileManager, &currentNode->item);
        currentNode = currentNode->next_brother;
    }
    printf("[LOG] Semua file di direktori saat ini telah dipilih\n");
}
// === UNDO/REDO

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void undo(FileManager* fileManager) {
    Operation* operationToUndo;
    Operation* operationToRedo;
    Tree foundTree;
    TrashItem* trashItem;
    if (fileManager->undo == NULL) {
        printf("[LOG] No actions to undo.\n");
        return;
    }
    operationToUndo = alloc(Operation);
    operationToRedo = alloc(Operation);
    operationToUndo = (Operation*)pop(&(fileManager->undo));

    *operationToRedo = createOperation(operationToUndo->from, operationToUndo->to, operationToUndo->type, operationToUndo->isDir, NULL);

    switch (operationToUndo->type) {
    case ACTION_CREATE:
        printf("[LOG] Undo Path: %s\n", operationToUndo->from);
        // Hapus item yang baru dibuat
        foundTree = searchTree(fileManager->root, createItem(_getNameFromPath(operationToUndo->from), operationToUndo->from, 0, ITEM_FILE, 0, 0, 0));
        if (foundTree != NULL) {
            operationToUndo->isDir = foundTree->item.type == ITEM_FILE ? false : true;
            _deleteSingleItem(foundTree->item.path, foundTree->item.type, foundTree->item.name);
            printf("[LOG] Undo create: %s\n", operationToUndo->from);
        }
        else {
            printf("[LOG] Item tidak ditemukan untuk dihapus: %s\n", operationToUndo->from);
        }
        break;
    case ACTION_DELETE:
        operationToRedo->itemTemp = alloc(Queue);
        create_queue(&(*(operationToRedo->itemTemp)));
        // Kembalikan item yang dihapus
        if (is_queue_empty(*(operationToUndo->itemTemp))) {
            printf("bajigur, kosong\n");
        }
        while (!is_queue_empty(*(operationToUndo->itemTemp))) {
            trashItem = (TrashItem*)dequeue(&(*operationToUndo->itemTemp));
            printf("Item origin path:%s\n", trashItem->originalPath);
            enqueue(&(*operationToRedo->itemTemp), trashItem);
            // Buat item baru dengan path yang sudah dihapus
            createFile(fileManager, trashItem->item.type, _getDirectoryFromPath(trashItem->originalPath), trashItem->item.name, false);

            // enqueue(&(operationToRedo->itemTemp), trashItem);
        }
        printf("undo delete selesai\n");
        break;
    case ACTION_UPDATE:
        // Kembalikan nama item yang diubah
        foundTree = searchTree(fileManager->root, createItem(_getNameFromPath(operationToUndo->to), operationToUndo->to, 0, ITEM_FILE, 0, 0, 0));
        if (foundTree != NULL) {
            renameFile(fileManager, foundTree->item.path, _getNameFromPath(operationToUndo->from), false);
            printf("[LOG] Undo update: %s to %s\n", operationToUndo->to, operationToUndo->from);
        }
        break;
    case ACTION_RECOVER:
        // Hapus item yang sudah di-recover
        foundTree = searchTree(fileManager->root, createItem(_getNameFromPath(operationToUndo->from), operationToUndo->from, 0, ITEM_FILE, 0, 0, 0));
        if (foundTree != NULL) {
            _moveToTrash(fileManager, foundTree);
            printf("[LOG] Undo recover: %s\n", operationToUndo->from);
        }
        break;
    case ACTION_PASTE:
        // Hapus item yang sudah di-paste
        break;

    default:
        break;
    }
    push(&(fileManager->redo), operationToRedo);
    refreshFileManager(fileManager);
    // printf("[LOG] Redo Pushed : %s\n", operationToUndo->from);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void redo(FileManager* fileManager) {
    Operation* operationToRedo;
    Operation* operationToUndo;
    Tree foundTree;
    if (fileManager->redo == NULL) {
        printf("[LOG] No actions to redo.\n");
        return;
    }
    operationToRedo = alloc(Operation);
    operationToRedo = (Operation*)pop(&(fileManager->redo));
    operationToUndo = alloc(Operation);
    *operationToUndo = (Operation){
        .from = strdup(operationToRedo->from),
        .isDir = operationToRedo->isDir,
        .to = strdup(operationToRedo->to),
        .type = operationToRedo->type,
        .itemTemp = NULL };
    switch (operationToRedo->type) {
    case ACTION_CREATE:
        // Buat item yang sudah dihapus
        createFile(fileManager, operationToRedo->isDir ? ITEM_FOLDER : ITEM_FILE, _getDirectoryFromPath(operationToRedo->from), _getNameFromPath(operationToRedo->from), false);
        printf("[LOG] Redo create: %s\n", operationToRedo->from);
        break;
    case ACTION_DELETE:
        operationToUndo->itemTemp = alloc(Queue);
        create_queue(&(*(operationToUndo->itemTemp)));
        // kembalikan item ke trash (delete to trash)
        while (!is_queue_empty(*(operationToRedo->itemTemp))) {
            TrashItem* trashItem = (TrashItem*)dequeue(&(*operationToRedo->itemTemp));
            enqueue(&(*operationToUndo->itemTemp), trashItem);
            printf("[LOG] Redo delete item: %s\n", trashItem->item.name);
            foundTree = searchTree(fileManager->root, createItem(trashItem->item.name, trashItem->item.path, 0, trashItem->item.type, 0, 0, 0));
            if (foundTree == NULL) {
                printf("[LOG] Item tidak ditemukan untuk di-delete kembali: %s\n", trashItem->item.name);
                continue;
            }
            _moveToTrash(fileManager, foundTree);
            printf("[LOG] Redo delete: %s\n", trashItem->item.name);
        }

        printf("[LOG] Redo delete completed\n");
        break;
    case ACTION_UPDATE:
        foundTree = searchTree(fileManager->root, createItem(_getNameFromPath(operationToRedo->from), operationToRedo->from, 0, ITEM_FILE, 0, 0, 0));
        renameFile(fileManager, foundTree->item.path, _getNameFromPath(operationToRedo->to), false);
        printf("[LOG] Redo rename: %s\n", operationToRedo->from);
        break;
    case ACTION_PASTE:
        break;

    case ACTION_RECOVER:
        // 1. Ambil item yang sebelumnya telah dihapus
        // 2. Recover item ke tempat asal
        break;
    }
    push(&(fileManager->undo), operationToUndo);
}

// ================================================================================
// . . . HELPER FUNC / PROC . . .
// ================================================================================

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
char* _getNameFromPath(char* path) {
    char* name = strrchr(path, '/'); // dapatkan string yang dimulai dari karakter slash (/) terakhir
    if (name != NULL) {
        return name + 1; // skip karakter slash (/) terakhir
    }
    return path; // kembalikan pathnya kalau gak ada slash (/) (ini berarti sudah nama file)
};

/*  Prosedur helper untuk remove dari LinkedList trash
 *  IS:
 *  FS:
================================================================================*/
void _removeFromTrash(FileManager* fileManager, char* itemName) {
    Node* current = fileManager->trash.head;
    Node* prev = NULL;

    while (current != NULL) {
        TrashItem* trashItem = (TrashItem*)current->data;
        if (strcmp(trashItem->item.name, itemName) == 0) {
            if (prev == NULL) {
                fileManager->trash.head = current->next;
            }
            else {
                prev->next = current->next;
            }

            free(trashItem->originalPath);
            free(trashItem->trashPath);
            free(trashItem);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

/*  Prosedur helper untuk add back to tree
 *  IS:
 *  FS:
================================================================================*/
void _addBackToTree(FileManager* fileManager, TrashItem* trashItem, char* recoverPath) {
    // Update item dengan path baru
    Item recoveredItem = trashItem->item;
    recoveredItem.path = strdup(fileManager->currentPath);
    recoveredItem.name = strdup(_getNameFromPath(recoverPath));
    recoveredItem.deleted_at = 0; // Reset deleted time

    // Cari parent node di tree
    Tree parentNode = searchTree(fileManager->root,
        createItem(_getNameFromPath(fileManager->currentPath),
            fileManager->currentPath, 0, ITEM_FOLDER, 0, 0, 0));

    if (parentNode != NULL) {
        // Insert ke tree sebagai child dari current directory
        insert_node(parentNode, recoveredItem);
        printf("[LOG] Item berhasil ditambahkan kembali ke tree\n");
    }
    else {
        printf("[LOG] Gagal menemukan parent directory di tree\n");
    }
}

/*  Prosedur untuk hapus permanen (untuk undo operations)
 *  IS:
 *  FS:
================================================================================*/
void _deletePermanently(char* fullPath, ItemType type, char* name) {
    if (type == ITEM_FOLDER) {
        if (RemoveItemsRecurse(fullPath) != 0) {
            printf("[LOG] Gagal menghapus folder %s\n", name);
        }
    }
    else {
        if (remove(fullPath) != 0) {
            printf("[LOG] Gagal menghapus file %s\n", name);
        }
    }
}

/*  Prosedur Fungsi helper untuk cut operation (hapus permanen)
 *  IS:
 *  FS:
================================================================================*/
void _deleteSingleItem(char* fullPath, ItemType type, char* name) {
    // Cut operation = hapus permanen (tidak ke trash)
    _deletePermanently(fullPath, type, name);
}

// Prosedur untuk remove node dari tree
void remove_node(Tree* root, Tree nodeToRemove) {
    if (*root == NULL || nodeToRemove == NULL)
        return;

    Tree parent = nodeToRemove->parent;

    if (parent == NULL) {
        *root = NULL;
        free(nodeToRemove);
        return;
    }

    if (parent->first_son == nodeToRemove) {
        parent->first_son = nodeToRemove->next_brother;
    }
    else {
        Tree sibling = parent->first_son;
        while (sibling != NULL && sibling->next_brother != nodeToRemove) {
            sibling = sibling->next_brother;
        }

        if (sibling != NULL) {
            sibling->next_brother = nodeToRemove->next_brother;
        }
    }

    free(nodeToRemove);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void _copyFileContent(char* srcPath, char* destPath) {
    FILE* src = fopen(srcPath, "rb");
    FILE* dest = fopen(destPath, "wb");

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

void _moveToTrash(FileManager* fileManager, Tree itemTree) {
    char* trashDir = ".dir/trash";
    if (!DirectoryExists(trashDir)) {
        MakeDirectory(trashDir);
    }

    // Buat TrashItem dengan metadata
    TrashItem* trashItem = (TrashItem*)malloc(sizeof(TrashItem));
    trashItem->item = itemTree->item;
    trashItem->originalPath = strdup(itemTree->item.path);
    trashItem->deletedTime = time(NULL);

    char* srcPath = itemTree->item.path;
    char* trashPath = TextFormat("%s/%s", trashDir, itemTree->item.name);

    // Handle nama duplikat di trash
    if (FileExists(trashPath) || DirectoryExists(trashPath)) {
        if (itemTree->item.type == ITEM_FOLDER) {
            trashPath = _createDuplicatedFolderName(trashPath, "(1)");
        }
        else {
            trashPath = _createDuplicatedFileName(trashPath, "(1)");
        }
    }

    trashItem->trashPath = strdup(trashPath);

    // Move ke trash secara fisik
    if (rename(srcPath, trashPath) != 0) {
        printf("[LOG] Gagal memindahkan %s ke trash\ntrashPath:%s\nsrchPath:%s\n", itemTree->item.name, trashPath, srcPath);
    }

    // Tambahkan ke LinkedList trash (bukan Tree)
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = trashItem;
    newNode->next = fileManager->trash.head;
    fileManager->trash.head = newNode;

    // Hapus dari tree utama
    remove_node(&(fileManager->root), itemTree);
}

/* FungsProseduri rekursif untuk copy folder
 *  IS:
 *  FS:
================================================================================*/
void _copyFolderRecursive(char* srcPath, char* destPath) {
    DIR* dp = opendir(srcPath);
    if (!dp) {
        printf("[LOG] Gagal membuka direktori source: %s\n", srcPath);
        return;
    }

    struct dirent* ep;
    struct stat statbuf;

    printf("[LOG] Copying dari %s ke %s\n", srcPath, destPath);

    while ((ep = readdir(dp)) != NULL) {
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;

        // Fix: Use manual string construction instead of TextFormat
        char* srcItem = malloc(strlen(srcPath) + strlen(ep->d_name) + 2);
        char* destItem = malloc(strlen(destPath) + strlen(ep->d_name) + 2);

        sprintf(srcItem, "%s/%s", srcPath, ep->d_name);
        sprintf(destItem, "%s/%s", destPath, ep->d_name);

        printf("[LOG] Processing: %s -> %s\n", srcItem, destItem);

        if (stat(srcItem, &statbuf) == -1) {
            printf("[LOG] Gagal stat file: %s\n", srcItem);
            free(srcItem);
            free(destItem);
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // Buat folder dan copy rekursif
            printf("[LOG] Membuat direktori: %s\n", destItem);
            if (MakeDirectory(destItem) != 0) {
                printf("[LOG] Gagal membuat direktori: %s\n", destItem);
                free(srcItem);
                free(destItem);
                continue;
            }
            _copyFolderRecursive(srcItem, destItem);
        }
        else if (S_ISREG(statbuf.st_mode)) {
            // Copy file content
            printf("[LOG] Copying file: %s -> %s\n", srcItem, destItem);
            _copyFileContent(srcItem, destItem);
        }

        free(srcItem);
        free(destItem);
    }
    closedir(dp);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
bool isDirectory(char* path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
char* _createDuplicatedFolderName(char* filePath, char* suffix) {
    char* newPath = TextFormat("%s%s", filePath, suffix);
    if (DirectoryExists(newPath)) {
        newPath = _createDuplicatedFolderName(newPath, suffix);
    }
    return newPath;
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
char* _createDuplicatedFileName(char* filePath, char* suffix) {
    size_t len;
    char* extention = strrchr(filePath, '.');
    if (extention) {
        len = extention - filePath;
    }
    else {
        len = strlen(filePath);
    }
    char* nameOnly = (char*)malloc(len + 1);
    strncpy(nameOnly, filePath, len);
    nameOnly[len] = '\0';

    char* newPath = TextFormat("%s%s%s", nameOnly, suffix, extention);
    if (FileExists(newPath)) {
        newPath = _createDuplicatedFileName(newPath, suffix);
    }
    return newPath;
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void windowsOpenWith(char* path) {
    printf("[LOG] %s\n", path);

    char* command = "cmd /c start \"\"";
    int length = strlen(command) + strlen(path) + 5;

    char* executeableCommand = malloc(length);

    printf("[LOG] %d\n", length);

    snprintf(executeableCommand, length, "%s \"%s\" /OPENAS", command, path);

    printf("[LOG] %s\n", executeableCommand);

    system(executeableCommand);

    free(executeableCommand);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
Tree getCurrentRoot(FileManager fileManager) {
    if (fileManager.treeCursor == NULL) {
        return NULL;
    }

    Tree currentRoot = fileManager.treeCursor;
    while (currentRoot->parent != NULL) {
        currentRoot = currentRoot->parent;
    }
    return currentRoot;
}

char* getCurrentPath(Tree tree) {
    char* path = strdup("");
    if (!path)
        return NULL;

    while (tree != NULL) {
        char* name = tree->item.name;
        size_t newLen = strlen(name) + strlen(path) + 2;

        char* newPath = malloc(newLen);
        if (!newPath) {
            free(path);
            return NULL;
        }

        if (tree->parent == NULL) {
            snprintf(newPath, newLen, "%s%s", name, path);
        }
        else {
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
void goTo(FileManager* fileManager, Tree tree) {
    if (!fileManager || !tree)
        return;

    fileManager->treeCursor = tree;

    char* newPath = getCurrentPath(tree);
    if (newPath) {
        // free(fileManager->currentPath);
        fileManager->currentPath = newPath;
    }

    refreshFileManager(fileManager);

    printf("[LOG] %s\n", newPath);
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void goBack(FileManager* fileManager) {
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
void sort_children(Tree* parent) {
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
        }
        else {
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

char* _getDirectoryFromPath(char* path) {
    char* lastSlash = strrchr(path, '/');
    if (lastSlash == NULL) {
        return strdup("");
    }

    size_t len = lastSlash - path;
    char* dirPath = malloc(len + 1);

    if (dirPath == NULL) {
        perror("malloc failed");
        exit(1);
    }

    strncpy(dirPath, path, len);
    dirPath[len] = '\0';

    return dirPath;
}

/*  Function untuk mengecek apakah perlu menampilkan progress bar
 *  IS: totalItems diketahui
 *  FS: return true jika perlu progress bar, false jika tidak
================================================================================*/
bool shouldShowProgressBar(int totalItems) {
    // Tampilkan progress bar jika item lebih dari 10
    return totalItems > 10;
}

/*  Function check if path is external
 *  IS: Path diketahui
 *  FS: Return true jika path di luar workspace, false jika dalam workspace
================================================================================*/
bool isExternalPath(char* path) {
    if (path == NULL) return false;

    // Normalisasi path untuk perbandingan
    char* normalizedPath = strdup(path);

    // Convert backslash ke forward slash untuk Windows
    for (int i = 0; normalizedPath[i]; i++) {
        if (normalizedPath[i] == '\\') {
            normalizedPath[i] = '/';
        }
    }

    // Cek apakah path dimulai dengan workspace directory (.dir)
    bool isExternal = strncmp(normalizedPath, ".dir", 4) != 0 &&
        strncmp(normalizedPath, "./.dir", 6) != 0;

    free(normalizedPath);
    return isExternal;
}

// ...existing code...

/*  Function validate import path menggunakan Windows utils
 *  IS: Path yang akan diimpor diketahui
 *  FS: Return true jika valid, false jika tidak
================================================================================*/
bool validateImportPath(char* path) {
    if (path == NULL || strlen(path) == 0) {
        return false;
    }

    // Cek apakah path eksternal (di luar workspace)
    if (!isExternalPath(path)) {
        return false;
    }

    // Gunakan Windows utils untuk validasi
    return ValidateWindowsPath(path) == 1;
}


/*  Function check if path is directory menggunakan Windows utils
 *  IS: Path diketahui
 *  FS: Return true jika directory, false jika file
================================================================================*/

// ...existing code...

/*  Prosedur import file/folder dengan progress tracking
 *  IS: sourcePath valid dan di luar workspace
 *  FS: File/folder diimpor ke direktori saat ini dengan progress feedback
================================================================================*/
void importFile(FileManager* fileManager, char* sourcePath, bool isOperation) {
    sourcePath = _convertToUnixPath(sourcePath);
    if (!validateImportPath(sourcePath)) {
        printf("[LOG] Path tidak valid untuk import: %s\n", sourcePath);
        return;
    }

    if (fileManager->treeCursor == NULL) {
        printf("[LOG] Tidak ada direktori tujuan yang aktif\n");
        return;
    }

    printf("[LOG] Memulai import dari: %s\n", sourcePath);
    printf("[LOG] Ke direktori: %s\n", fileManager->treeCursor->item.path);

    // Dapatkan nama file/folder dari path
    char* fileName = _getNameFromPath(sourcePath);
    if (fileName == NULL || strlen(fileName) == 0) {
        printf("[LOG] Tidak dapat mengekstrak nama file dari path\n");
        return;
    }

    // Buat path tujuan di direktori saat ini
    char* destinationPath = TextFormat("%s/%s", fileManager->treeCursor->item.path, fileName);

    // Handle duplikasi nama dengan suffix yang lebih deskriptif
    if (FileExists(destinationPath) || DirectoryExists(destinationPath)) {
        char* timestamp = TextFormat("_imported_%ld", time(NULL) % 10000);
        if (isDirectory(sourcePath)) {
            destinationPath = _createDuplicatedFolderName(destinationPath, timestamp);
        }
        else {
            destinationPath = _createDuplicatedFileName(destinationPath, timestamp);
        }
        printf("[LOG] File exists, using new name: %s\n", _getNameFromPath(destinationPath));
    }

    // Tentukan tipe item dan size untuk progress
    ItemType itemType = isDirectory(sourcePath) ? ITEM_FOLDER : ITEM_FILE;
    bool success = false;

    printf("[LOG] Tipe item: %s\n", itemType == ITEM_FOLDER ? "Folder" : "File");

    // Lakukan operasi copy dengan progress tracking
    if (itemType == ITEM_FOLDER) {
        printf("[LOG] Mengimpor folder...\n");

        // Buat direktori tujuan
        if (MakeDirectory(destinationPath) == 0) {
            printf("[LOG] Direktori tujuan dibuat: %s\n", destinationPath);

            // Copy semua isi folder secara rekursif dengan progress
            printf("[LOG] Memulai copy rekursif...\n");
            _copyFolderRecursive(sourcePath, destinationPath);

            success = true;
            printf("[LOG] Folder berhasil diimpor: %s\n", fileName);
        }
        else {
            printf("[LOG] Gagal membuat direktori tujuan: %s\n", destinationPath);
        }
    }
    else {
        printf("[LOG] Mengimpor file...\n");

        // Copy file dengan progress feedback
        _copyFileContent(sourcePath, destinationPath);

        if (FileExists(destinationPath)) {
            success = true;
            printf("[LOG] File berhasil diimpor: %s\n", fileName);
        }
        else {
            printf("[LOG] Gagal mengimpor file: %s\n", fileName);
        }
    }

    if (success) {
        printf("[LOG] Import berhasil, menambahkan ke tree...\n");

        // Tambahkan ke tree structure
        struct stat statbuf;
        if (stat(destinationPath, &statbuf) == 0) {
            Item newItem = createItem(
                _getNameFromPath(destinationPath),
                destinationPath,
                statbuf.st_size,
                itemType,
                statbuf.st_ctime,
                time(NULL),
                0
            );

            Tree newNode = insert_node(fileManager->treeCursor, newItem);

            // Jika folder, load strukturnya
            if (itemType == ITEM_FOLDER && newNode != NULL) {
                printf("[LOG] Loading tree structure untuk folder...\n");
                loadTree(newNode, destinationPath);
            }

            // Simpan operasi untuk undo jika diminta
            if (isOperation) {
                Operation* importOperation = alloc(Operation);
                *importOperation = createOperation(destinationPath, sourcePath, ACTION_CREATE, (itemType == ITEM_FOLDER), NULL);
                push(&fileManager->undo, importOperation);
                printf("[LOG] Operasi import disimpan untuk undo\n");
            }

            // Refresh file manager untuk update UI
            refreshFileManager(fileManager);

            printf("[LOG] Import selesai berhasil: %s -> %s\n", sourcePath, destinationPath);
        }
        else {
            printf("[LOG] Gagal mendapatkan stat untuk file yang diimpor\n");
        }
    }
    else {
        printf("[LOG] Import gagal untuk: %s\n", sourcePath);
    }
}

char *_convertToUnixPath(char *path) {
    if (path == NULL) return NULL;

    // Alokasikan memori untuk path baru
    char *unixPath = malloc(strlen(path) + 1);
    if (unixPath == NULL) {
        perror("malloc failed");
        exit(1);
    }

    // Ganti backslash dengan forward slash
    for (int i = 0; path[i] != '\0'; i++) {
        unixPath[i] = (path[i] == '\\') ? '/' : path[i];
    }
    unixPath[strlen(path)] = '\0'; // Tambahkan null terminator

    return unixPath;
}
