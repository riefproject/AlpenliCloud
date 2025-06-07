#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>

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
#include "gui/navbar.h"
#include "gui/sidebar.h"

#include <time.h>
#define _DIR ".dir/"
#define ROOT ".dir/root"
#define TRASH ".dir/trash"
#define TRASH_DUMP ".dir/.trash"

bool isCopy = 0;
char* _generateUID();
void _removeFromTrashByUID(FileManager* fileManager, char* uid);
void _addBackToTreeFromTrash(FileManager* fileManager, TrashItem* trashItem, char* recoverPath);
void _addTreeStructureRecursive(Tree parentNode, char* folderPath);
void _debugPrintTreeStructure(Tree node, int depth);

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
    create_queue(&(fileManager->clipboard));
    create_list(&(fileManager->selectedItem));
    create_list(&(fileManager->searchingList));
    fileManager->isRootTrash = false;
    fileManager->isSearching = false;
    fileManager->needsRefresh = false;
    fileManager->currentPath = NULL;
    fileManager->treeCursor = NULL;
    fileManager->ctx = NULL;
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
 * IS: Direktori `path` valid dan dapat diakses. contoh path: `.dir/root` (dimulai dari ./dir)
 * FS: Tree diisi dengan struktur direktori dan file dari `path`.
 * Author: Farras Fadhil Syafiq
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
 * Format: UID,originalName,originalPath,deletedTime
 * Author: Farras Fadhil Syafiq
 * Edited by: Arief F-sa Wijaya
 ================================================================================*/
void loadTrashFromFile(LinkedList* trash) {
    FILE* trashFile = fopen(TRASH_DUMP, "r");
    if (trashFile == NULL) {
        printf("[LOG] File trash tidak ditemukan, membuat baru\n");
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

            // Parse format: UID,originalName,originalPath,deletedTime
            char* uid = strtok(line, ",");
            char* originalName = strtok(NULL, ",");
            char* originalPath = strtok(NULL, ",");
            char* deletedTimeStr = strtok(NULL, ",");

            if (uid && originalName && originalPath && deletedTimeStr) {
                TrashItem* trashItem = alloc(TrashItem);
                if (!trashItem) {
                    len = 0;
                    continue;
                }

                // Set data dengan nama asli untuk display
                trashItem->uid = strdup(uid);
                trashItem->item.name = strdup(originalName);  // Nama asli untuk display
                trashItem->originalPath = strdup(originalPath);
                trashItem->deletedTime = (time_t)atoll(deletedTimeStr);

                // Set tipe item berdasarkan ekstensi atau asumsi
                char* fileExtension = strrchr(originalName, '.');
                trashItem->item.type = fileExtension ? ITEM_FILE : ITEM_FOLDER;

                // Konstruksi trash path berdasarkan UID
                char* trashFileName;
                if (fileExtension && trashItem->item.type == ITEM_FILE) {
                    size_t nameLen = fileExtension - originalName;
                    char* nameWithoutExt = malloc(nameLen + 1);
                    strncpy(nameWithoutExt, originalName, nameLen);
                    nameWithoutExt[nameLen] = '\0';

                    trashFileName = TextFormat("%s_%s%s", nameWithoutExt, uid, fileExtension);
                    free(nameWithoutExt);
                }
                else {
                    trashFileName = TextFormat("%s_%s", originalName, uid);
                }

                trashItem->trashPath = strdup(TextFormat("%s/%s", TRASH, trashFileName));

                // Set item properties untuk display
                trashItem->item.path = strdup(trashItem->trashPath);
                trashItem->item.size = 0; // Will be updated when needed
                trashItem->item.created_at = 0;
                trashItem->item.deleted_at = trashItem->deletedTime;
                trashItem->item.selected = false;

                insert_first(trash, trashItem);
                printf("[LOG] Loaded trash item: %s (UID: %s, Deleted: %ld)\n",
                    originalName, uid, trashItem->deletedTime);
            }

            len = 0;
        }
        else {
            line[len++] = ch;
        }
    }

    // Handle last line without newline
    if (len > 0) {
        line[len] = '\0';
        char* uid = strtok(line, ",");
        char* originalName = strtok(NULL, ",");
        char* originalPath = strtok(NULL, ",");
        char* deletedTimeStr = strtok(NULL, ",");

        if (uid && originalName && originalPath && deletedTimeStr) {
            TrashItem* trashItem = alloc(TrashItem);
            if (trashItem) {
                trashItem->uid = strdup(uid);
                trashItem->item.name = strdup(originalName);
                trashItem->originalPath = strdup(originalPath);
                trashItem->deletedTime = (time_t)atoll(deletedTimeStr);

                char* fileExtension = strrchr(originalName, '.');
                trashItem->item.type = fileExtension ? ITEM_FILE : ITEM_FOLDER;

                char* trashFileName;
                if (fileExtension && trashItem->item.type == ITEM_FILE) {
                    size_t nameLen = fileExtension - originalName;
                    char* nameWithoutExt = malloc(nameLen + 1);
                    strncpy(nameWithoutExt, originalName, nameLen);
                    nameWithoutExt[nameLen] = '\0';

                    trashFileName = TextFormat("%s_%s%s", nameWithoutExt, uid, fileExtension);
                    free(nameWithoutExt);
                }
                else {
                    trashFileName = TextFormat("%s_%s", originalName, uid);
                }

                trashItem->trashPath = strdup(TextFormat("%s/%s", TRASH, trashFileName));
                trashItem->item.path = strdup(trashItem->trashPath);
                trashItem->item.deleted_at = trashItem->deletedTime;

                insert_first(trash, trashItem);
                printf("[LOG] Loaded trash item: %s (UID: %s, Deleted: %ld)\n",
                    originalName, uid, trashItem->deletedTime);
            }
        }
    }

    free(line);
    fclose(trashFile);
}

/*
 * IS  : Antrian `fileManager->trash` sudah terisi.
 * FS  : Menyimpan isi antrian trash ke dalam file `TRASH_DUMP`.
 * Format: UID,originalName,originalPath,deletedTime
 * Author: Farras Fadhil Syafiq
 * Edited by: Arief F-sa Wijaya
 ================================================================================*/
void saveTrashToFile(FileManager* fileManager) {
    FILE* trashFile = fopen(TRASH_DUMP, "w");
    if (trashFile == NULL) {
        perror("[ERROR] Gagal membuka file trash untuk menulis");
        return;
    }

    Node* current = fileManager->trash.head;
    while (current != NULL) {
        TrashItem* trashItem = (TrashItem*)current->data;
        if (trashItem && trashItem->uid && trashItem->item.name && trashItem->originalPath) {
            // Format: UID,originalName,originalPath,deletedTime
            fprintf(trashFile, "%s,%s,%s,%ld\n",
                trashItem->uid,
                trashItem->item.name,        // Nama asli untuk display
                trashItem->originalPath,
                trashItem->deletedTime);     // Waktu dihapus
        }
        current = current->next;
    }

    fclose(trashFile);
    printf("[LOG] Trash data saved with UID as primary key and deleted time\n");
}
/*
 * IS: LinkedList trash berisi item yang telah dihapus.
 * FS: Mencetak semua item yang ada di LinkedList trash ke konsol dengan format yang lebih informatif.
 * Author: Farras Fadhil Syafiq
 * Edited by: Arief F-sa Wijaya
================================================================================*/
void printTrash(LinkedList trash) {
    Node* current = trash.head;
    if (current == NULL) {
        printf("[LOG] Trash is empty\n");
        return;
    }

    printf("\n=== TRASH CONTENTS ===\n");
    printf("%-20s %-30s %-50s %-20s %-15s\n",
        "UID", "Display Name", "Original Path", "Deleted Time", "Type");
    printf("%-20s %-30s %-50s %-20s %-15s\n",
        "----", "------------", "-------------", "------------", "----");

    while (current != NULL) {
        TrashItem* trashItem = (TrashItem*)current->data;
        if (trashItem != NULL) {
            // Convert deleted time to readable format
            struct tm* timeinfo = localtime(&trashItem->deletedTime);
            char timeStr[20];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

            printf("%-20s %-30s %-50s %-20s %-15s\n",
                trashItem->uid,
                trashItem->item.name,        // Display nama asli
                trashItem->originalPath,     // Path asal
                timeStr,                     // Waktu dihapus
                (trashItem->item.type == ITEM_FOLDER) ? "Folder" : "File");
        }
        current = current->next;
    }
    printf("======================\n\n");
}
/*
 * IS:
 FS:
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
    // Refresh sidebar dengan cara yang aman
    if (fileManager->ctx && fileManager->ctx->sidebar) {
        _refreshSidebarSafely(fileManager);
        printf("[LOG] Refreshing sidebar...\n");
        printf("[LOG] Sidebar refreshed successfully\n");
    }
    return;
    if (!fileManager || !fileManager->treeCursor) {
        printf("[LOG] Invalid fileManager or treeCursor in refresh\n");
        return;
    }

    // Add null checks for treeCursor->item.path
    if (!fileManager->treeCursor->item.path) {
        printf("[LOG] Invalid path in treeCursor\n");
        return;
    }

    printf("[LOG] Refreshing FileManager...\n");

    // PERBAIKAN: Simpan informasi treeCursor saat ini
    char* currentPath = strdup(fileManager->treeCursor->item.path);
    char* currentName = strdup(fileManager->treeCursor->item.name);

    // PERBAIKAN: Refresh dengan cara yang aman
    _refreshTreeSafely(fileManager, currentPath);

    // PERBAIKAN: Update treeCursor dengan node yang baru
    fileManager->treeCursor = _findNodeByPath(fileManager->root, currentPath);

    if (!fileManager->treeCursor) {
        printf("[LOG] Error: treeCursor tidak ditemukan setelah refresh\n");
        // Fallback ke root
        fileManager->treeCursor = fileManager->root;
    }

    printf("[LOG] Directory refreshed successfully\n");



    // Cleanup
    free(currentPath);
    free(currentName);
}

// Helper function untuk refresh tree secara aman
void _refreshTreeSafely(FileManager* fileManager, char* targetPath) {
    // Cari node target di tree
    Tree targetNode = _findNodeByPath(fileManager->root, targetPath);

    if (!targetNode) {
        printf("[LOG] Target node tidak ditemukan untuk refresh\n");
        return;
    }

    // Hapus hanya children dari target node
    if (targetNode->first_son) {
        destroyTree(&targetNode->first_son);
        targetNode->first_son = NULL;
    }

    // Load ulang children
    loadTree(targetNode, targetPath);
}

// Helper function untuk mencari node berdasarkan path
Tree _findNodeByPath(Tree root, char* targetPath) {
    if (!root || !targetPath) return NULL;

    // Jika path cocok, return node ini
    if (root->item.path && strcmp(root->item.path, targetPath) == 0) {
        return root;
    }

    // Cari di children
    Tree current = root->first_son;
    while (current) {
        Tree found = _findNodeByPath(current, targetPath);
        if (found) return found;
        current = current->next_brother;
    }

    return NULL;
}

// Helper function untuk refresh sidebar secara aman
void _refreshSidebarSafely(FileManager* fileManager) {
    if (!fileManager->ctx || !fileManager->ctx->sidebar) return;

    // Collect state sebelum destroy
    SidebarState* stateList = NULL;
    if (fileManager->ctx->sidebar->sidebarRoot) {
        collectSidebarState(fileManager->ctx->sidebar->sidebarRoot, &stateList);
        destroySidebarItem(&fileManager->ctx->sidebar->sidebarRoot);
        fileManager->ctx->sidebar->sidebarRoot = NULL;
    }

    // Rebuild dengan state yang tersimpan
    fileManager->ctx->sidebar->sidebarRoot = createSidebarItemWithState(fileManager->root, stateList);

    // Cleanup state
    if (stateList) {
        destroySidebarState(stateList);
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

/*  Prosedur untuk recover file berdasarkan UID
 *  IS: Item dipilih dari trash
 *  FS: Item dikembalikan ke originalPath atau current directory
================================================================================*/
void recoverFile(FileManager* fileManager) {
    if (fileManager->selectedItem.head == NULL) {
        printf("[LOG] Tidak ada file yang dipilih untuk di-recover\n");
        return;
    }
    Operation* recoverOperation = alloc(Operation);
    *recoverOperation = createOperation(NULL, NULL, ACTION_RECOVER, false, NULL);
    recoverOperation->itemTemp = alloc(Queue);
    create_queue(&(*(recoverOperation->itemTemp)));
    Node* temp = fileManager->selectedItem.head;
    while (temp != NULL) {
        Item* itemToRecover = (Item*)temp->data;

        // Cari di LinkedList trash berdasarkan UID atau nama display
        Node* trashNode = fileManager->trash.head;
        TrashItem* foundTrashItem = NULL;

        while (trashNode != NULL) {
            TrashItem* trashItem = (TrashItem*)trashNode->data;
            // Match berdasarkan nama display dan path untuk identifikasi unik
            if (strcmp(trashItem->item.name, itemToRecover->name) == 0 &&
                strcmp(trashItem->item.path, itemToRecover->path) == 0) {
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

        // Tentukan path recovery - ke original path
        char* recoverPath = strdup(foundTrashItem->originalPath);

        // Handle nama duplikat di lokasi recovery
        if (FileExists(recoverPath) || DirectoryExists(recoverPath)) {
            char* dirPath = _getDirectoryFromPath(recoverPath);
            char* originalName = foundTrashItem->item.name;

            if (foundTrashItem->item.type == ITEM_FOLDER) {
                recoverPath = TextFormat("%s/%s(recovered)", dirPath, originalName);
                if (DirectoryExists(recoverPath)) {
                    recoverPath = _createDuplicatedFolderName(recoverPath, "(1)");
                }
            }
            else {
                recoverPath = _createDuplicatedFileName(recoverPath, "(recovered)");
            }
        }

        // Simpan operasi untuk undo
        enqueue(&(*recoverOperation->itemTemp), foundTrashItem);

        // Move dari trash ke lokasi recovery
        if (rename(foundTrashItem->trashPath, recoverPath) == 0) {
            printf("[LOG] File '%s' berhasil di-recover ke %s (UID: %s)\n",
                foundTrashItem->item.name, recoverPath, foundTrashItem->uid);

            // Hapus dari LinkedList trash
            _removeFromTrashByUID(fileManager, foundTrashItem->uid);

            // Tambah kembali ke tree dengan path recovery
            _addBackToTreeFromTrash(fileManager, foundTrashItem, recoverPath);
        }
        else {
            printf("[LOG] Gagal me-recover file %s (UID: %s)\n",
                foundTrashItem->item.name, foundTrashItem->uid);
        }

        temp = temp->next;
    }

    clearSelectedFile(fileManager);

    // Save perubahan trash
    saveTrashToFile(fileManager);
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

void searchingTreeItem(FileManager* fileManager, char* keyword) {
    if (fileManager->searchingList.head != NULL) {
        destroy_list(&(fileManager->searchingList));
        printf("[LOG] Hasil pencarian sebelumnya telah dihapus\n");
    }

    if (keyword == NULL || strlen(keyword) == 0) {
        printf("[LOG] Keyword pencarian tidak boleh kosong\n");
        return;
    }

    searchingTreeItemRecursive(&(fileManager->searchingList), fileManager->treeCursor, keyword);

    if (fileManager->searchingList.head == NULL) {
        printf("[LOG] Tidak ada file yang cocok dengan keyword '%s'\n", keyword);
    }
    else {
        printf("[LOG] Pencarian selesai. Gunakan printSearchingList() untuk melihat hasil.\n");
    }
}

void searchingTreeItemRecursive(LinkedList* linkedList, Tree tree, const char* keyword) {
    if (tree == NULL)
        return;

    if (strstr(tree->item.name, keyword) != NULL) {
        printf("===========================================================\n");
        insert_last(linkedList, tree);
        printf("[LOG] Menemukan item: %s, Path: %s\n", tree->item.name, tree->item.path);
        printf("===========================================================\n");
    }

    searchingTreeItemRecursive(linkedList, tree->first_son, keyword);
    searchingTreeItemRecursive(linkedList, tree->next_brother, keyword);
}

void searchingLinkedListItem(FileManager* fileManager, Node* node, char* keyword) {
    if (node == NULL) {
        return;
    }

    if (fileManager->searchingList.head != NULL) {
        destroy_list(&(fileManager->searchingList));
        printf("[LOG] Hasil pencarian sebelumnya telah dihapus\n");
    }

    if (keyword == NULL || strlen(keyword) == 0) {
        printf("[LOG] Keyword pencarian tidak boleh kosong\n");
        return;
    }

    searchingLinkedListRecursive(fileManager, node, keyword);

    if (fileManager->searchingList.head == NULL) {
        printf("[LOG] Tidak ada item yang cocok dengan keyword '%s'\n", keyword);
    }
    else {
        printf("[LOG] Pencarian selesai. Gunakan printSearchingList() untuk melihat hasil.\n");
    }
}

void searchingLinkedListRecursive(FileManager* fileManager, Node* node, char* keyword) {
    if (node == NULL)
        return;

    Tree current = (Tree)node->data; // diasumsikan node->data menyimpan pointer ke Tree
    if (strstr(current->item.name, keyword) != NULL) {
        printf("===========================================================\n");
        printf("[LOG] Menemukan item: %s, Path: %s\n", current->item.name, current->item.path);
        insert_last(&(fileManager->searchingList), current);
        printf("===========================================================\n");
    }

    searchingLinkedListRecursive(fileManager, node->next, keyword);
}

void printSearchingList(FileManager* fileManager) {
    if (fileManager->searchingList.head == NULL) {
        printf("[LOG] Tidak ada hasil pencarian\n");
        return;
    }

    Node* temp = fileManager->searchingList.head;
    while (temp != NULL) {
        Tree treePtr = (Tree)temp->data;
        printf("[LOG] Hasil Pencarian: %s, Path: %s, Size: %ld, Type: %s\n",
            treePtr->item.name,
            treePtr->item.path,
            treePtr->item.size,
            (treePtr->item.type == ITEM_FOLDER) ? "Folder" : "File");
        temp = temp->next;
    }
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void copyToClipboard(FileManager* fm) {
    // Clear clipboard yang lama
    if (!is_queue_empty(fm->clipboard)) {
        clear_queue(&fm->clipboard);
        printf("[LOG] Clipboard lama telah dibersihkan\n");
    }

    // Clear copied queue untuk memastikan konsistensi
    if (!is_queue_empty(fm->copied)) {
        clear_queue(&fm->copied);
        printf("[LOG] Copied queue telah dibersihkan\n");
    }

    // Copy item yang selected ke clipboard
    Node* temp = fm->selectedItem.head;
    int copiedCount = 0;

    while (temp != NULL) {

        Item* itemToCopy = (Item*)temp->data;

        // Enqueue ke clipboard (bukan ke copied)
        enqueue(&(fm->clipboard), itemToCopy);
        copiedCount++;

        printf("[LOG] Item %s ditambahkan ke clipboard\n", itemToCopy->name);
        temp = temp->next;
    }

    if (copiedCount == 0) {
        printf("[LOG] Gagal Menyalin File - tidak ada item yang dipilih!\n");
        return;
    }

    printf("[LOG] %d file berhasil disalin ke clipboard\n", copiedCount);
}

void copyFile(FileManager* fm) {
    copyToClipboard(fm);
    isCopy = true;
    printf("[LOG] Mode: COPY\n");
}

void cutFile(FileManager* fm) {
    copyToClipboard(fm);
    isCopy = false;
    printf("[LOG] Mode: CUT\n");
}

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/

// Function untuk menghitung total items dalam clipboard
int _calculateTotalPasteItems(FileManager* fileManager) {
    int totalItems = 0;
    Node* countNode = fileManager->clipboard.front;
    while (countNode != NULL) {
        totalItems++;
        countNode = countNode->next;

    }
    return totalItems;
}

// Function untuk memproses paste operation untuk satu item
bool _processSinglePasteItem(FileManager* fileManager, Item* itemToPaste, char** originPath) {
    // Path untuk file/folder baru di lokasi tujuan
    char* destinationFullPath = TextFormat("%s%s", _DIR, fileManager->currentPath);
    char* newPath = TextFormat("%s/%s", destinationFullPath, itemToPaste->name);
    *originPath = itemToPaste->path;

    printf("[LOG] Mencoba paste: %s -> %s\n", *originPath, newPath);
    printf("[LOG] Is Copy: %s\n", isCopy ? "true" : "false");
    printf("[LOG] Item type: %d (ITEM_FOLDER=%d, ITEM_FILE=%d)\n", itemToPaste->type, ITEM_FOLDER, ITEM_FILE);

    // COPY operation - update origin path
    if (isCopy) {
        Tree foundTree = searchTree(fileManager->root, *itemToPaste);
        if (foundTree == NULL) {
            printf("[LOG] File tidak ditemukan untuk copy: %s\n", itemToPaste->name);
            return false;
        }
        *originPath = foundTree->item.path;
        printf("[LOG] Origin path updated to: %s\n", *originPath);
    }

    // Cek apakah source file/folder masih ada
    if (!FileExists(*originPath) && !DirectoryExists(*originPath)) {
        printf("[LOG] Source tidak ditemukan: %s\n", *originPath);
        return false;
    }

    printf("[LOG] Source exists, proceeding with copy/cut\n");
    return true;
}


// Function untuk memproses paste folder
bool _pasteFolderItem(FileManager* fileManager, Item* itemToPaste, char* originPath, char* newPath) {
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
            return false;
        }
    }
    else {
        printf("[LOG] COPY operation - copying folder\n");
        printf("[LOG] Membuat folder destination: %s\n", newPath);
        if (MakeDirectory(newPath) != 0) {
            printf("[LOG] Gagal membuat folder\n");
            return false;
        }
        printf("[LOG] Folder destination created successfully\n");
        printf("[LOG] Mulai copy recursive dari %s ke %s\n", originPath, newPath);
        _copyFolderRecursive(originPath, newPath);
        printf("[LOG] Selesai copy recursive\n");
    }

    // Tambahkan ke tree struktur
    // _addItemToCurrentTree(fileManager, itemToPaste, newPath, ITEM_FOLDER);
    return true;
}

// Function untuk memproses paste file
bool _pasteFileItem(FileManager* fileManager, Item* itemToPaste, char* originPath, char* newPath) {
    printf("[LOG] Processing file: %s\n", itemToPaste->name);

    if (FileExists(newPath)) {
        newPath = _createDuplicatedFileName(newPath, "(1)");
    }

    if (!isCopy) {
        if (rename(originPath, newPath) != 0) {
            printf("[LOG] Gagal memindahkan file %s\n", itemToPaste->name);
            return false;
        }
    }
    else {
        _copyFileContent(originPath, newPath);
    }

    // Tambahkan item baru ke tree di lokasi tujuan
    // _addItemToCurrentTree(fileManager, itemToPaste, newPath, itemToPaste->type);
    return true;
}


// Helper function untuk menambahkan item ke current tree
void _addItemToCurrentTree(FileManager* fileManager, Item* itemToPaste, char* newPath, ItemType type) {
    Tree currentNode = fileManager->treeCursor;
    if (currentNode != NULL) {
        Item newItem = createItem(
            _getNameFromPath(newPath),
            newPath,
            itemToPaste->size,
            type,
            itemToPaste->created_at,
            time(NULL),
            0);
        insert_node(currentNode, newItem);
        printf("[LOG] Item %s berhasil ditambahkan ke tree\n", newItem.name);
    }
}

// Function untuk cleanup setelah cut operation
void _handleCutCleanup(FileManager* fileManager, Item* itemToPaste) {
    if (!isCopy) {
        Tree foundTree = searchTree(fileManager->root, *itemToPaste);
        if (foundTree != NULL) {
            remove_node(&(fileManager->root), foundTree);
            printf("[LOG] Item %s berhasil dihapus dari tree asal\n", itemToPaste->name);
        }
    }
}

// Function untuk membuat PasteItem
void _createPasteItemRecord(Item* itemToPaste, char* originPath) {
    PasteItem* pasteItem = alloc(PasteItem);
    *pasteItem = createPasteItem(*itemToPaste, originPath);
    printf("[LOG] PasteItem created for %s with original path %s\n", itemToPaste->name, originPath);
}

// Function utama yang sudah direfactor
void pasteFile(FileManager* fileManager, bool isOperation) {
    if (is_queue_empty(fileManager->clipboard)) {
        printf("[LOG] Clipboard kosong\n");
        return;
    }
    Operation* pasteOperation;
    PasteItem* pasteItem;
    if (isOperation) {
        pasteOperation = alloc(Operation);
        *pasteOperation = createOperation(NULL, NULL, ACTION_PASTE, false, NULL);
        pasteOperation->itemTemp = alloc(Queue);
        create_queue(&(*(pasteOperation->itemTemp)));
    }
    // Reset progress state
    resetProgressBarState();

    // Hitung total item untuk progress bar
    int totalItems = _calculateTotalPasteItems(fileManager);
    bool showProgress = shouldShowProgressBar(totalItems);
    int currentProgress = 0;
    bool cancelled = false;

    printf("[LOG] Total items to paste: %d, Show progress: %s\n",
        totalItems, showProgress ? "true" : "false");

    // PERBAIKAN: Buat temporary queue untuk iterasi
    Queue tempQueue;
    create_queue(&tempQueue);
    printf("[LOG] Temporary queue created for paste operation\n");
    // Copy semua items ke temporary queue
    Node* temp = fileManager->clipboard.front;

    while (temp != NULL) {
        Item* tempItem = alloc(Item);
        Item* container = (Item*)temp->data;
        tempItem->name = strdup(container->name);
        tempItem->path = strdup(container->path);
        tempItem->type = container->type;
        tempItem->selected = container->type;
        tempItem->created_at = container->created_at;
        tempItem->size = container->size;
        tempItem->updated_at = container->updated_at;
        tempItem->deleted_at = 0;
        printf("[LOG] Enqueueing item: %s to temporary queue\n", tempItem->name);
        enqueue(&tempQueue, tempItem);
        temp = temp->next;
    }

    // Iterasi menggunakan temporary queue
    while (!is_queue_empty(tempQueue) && !cancelled) {
        Item* itemToPaste = (Item*)dequeue(&tempQueue);
        if (itemToPaste == NULL) {
            printf("[LOG] Item to paste is NULL, skipping...\n");
            continue;
        }

        printf("[LOG] Processing item: %s\n", itemToPaste->name);
        // Update progress bar dan cek cancel
        if (showProgress) {
            showPasteProgressBar(currentProgress, totalItems, itemToPaste->name);
            if (shouldCancelPaste()) {
                printf("[LOG] Paste operation cancelled by user\n");
                cancelled = true;
                break;
            }
        }


        char* originPath;
        // Proses validasi dan setup untuk item ini
        if (!_processSinglePasteItem(fileManager, itemToPaste, &originPath)) {
            currentProgress++;
            continue;
        }


        // Path untuk file/folder baru di lokasi tujuan  
        char* destinationFullPath = TextFormat("%s%s", _DIR, fileManager->currentPath);
        char* newPath = TextFormat("%s/%s", destinationFullPath, itemToPaste->name);

        bool success = false;
        // Proses berdasarkan tipe item
        if (isOperation) {
            pasteOperation->isCopy = isCopy;
            printf("[LOG] Paste operation type: %s\n", isCopy ? "COPY" : "CUT");
        }
        if (itemToPaste->type == ITEM_FOLDER) {
            success = _pasteFolderItem(fileManager, itemToPaste, originPath, newPath);
        }
        else if (itemToPaste->type == ITEM_FILE) {
            success = _pasteFileItem(fileManager, itemToPaste, originPath, newPath);
        }

        // Di dalam pasteFile function, setelah success = true
        if (success) {
            _handleCutCleanup(fileManager, itemToPaste);

            // PERBAIKAN: Pastikan destinationFullPath benar
            Tree parentDestinationTree = searchTree(fileManager->root,
                createItem(_getNameFromPath(destinationFullPath), destinationFullPath, 0, ITEM_FOLDER, 0, 0, 0));

            if (parentDestinationTree != NULL) {
                // Tambahkan item utama
                Item newItem = createItem(
                    strdup(_getNameFromPath(newPath)),  // PERBAIKAN: strdup untuk safety
                    strdup(newPath),                    // PERBAIKAN: strdup untuk safety
                    itemToPaste->size,
                    itemToPaste->type,
                    itemToPaste->created_at,
                    time(NULL),
                    0);

                if (isOperation) {
                    pasteItem = alloc(PasteItem);
                    *pasteItem = createPasteItem(newItem, originPath);
                    printf("[LOG] Adding PasteItem for %s with original path %s\n", pasteItem->item.path, pasteItem->originalPath);
                    enqueue(&(*pasteOperation->itemTemp), pasteItem);
                }
                Tree newNode = insert_node(parentDestinationTree, newItem);
                printf("[LOG] Main item added: %s at %s\n", newItem.name, newItem.path);

                // Jika item yang di-paste adalah folder, tambahkan seluruh struktur dalamnya
                if (itemToPaste->type == ITEM_FOLDER && newNode != NULL) {
                    printf("[LOG] Starting recursive tree structure for folder: %s\n", newItem.name);
                    _addTreeStructureRecursive(newNode, newPath);
                    printf("[LOG] Completed recursive tree structure for folder: %s\n", newItem.name);
                }

                printf("[LOG] Item %s berhasil ditambahkan ke tree\n", newItem.name);
            }
            else {
                printf("[LOG] ERROR: Parent destination tree tidak ditemukan: %s\n", destinationFullPath);
                printf("[LOG] Searching for: name=%s, path=%s\n",
                    _getNameFromPath(destinationFullPath), destinationFullPath);
            }
        }
        currentProgress++;
    }

    // Cleanup temporary queue
    clear_queue(&tempQueue);

    // Reset progress state setelah selesai
    resetProgressBarState();

    // Clear clipboard setelah cut operation (hanya jika tidak di-cancel)
    if (!isCopy && !cancelled) {
        clear_queue(&fileManager->clipboard);
        printf("[LOG] Clipboard cleared after cut operation\n");
    }

    if (cancelled) {
        printf("[LOG] Paste operation was cancelled\n");
    }
    else {
        printf("[LOG] Paste berhasil!\n");
    }
    if (isOperation) {
        push(&fileManager->undo, pasteOperation);
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

// Helper function untuk mengecek apakah item sama
bool _isItemEqual(Item* item1, Item* item2) {
    return (strcmp(item1->path, item2->path) == 0 &&
        strcmp(item1->name, item2->name) == 0);
}

// Helper function untuk free node dan data
void _freeSelectedNode(Node* node) {
    if (node) {
        // Tidak perlu free item karena itu pointer ke item di tree
        free(node);
    }
}

void selectFile(FileManager* fileManager, Item* item) {
    // Cek apakah item sudah ada dalam list
    Node* temp = fileManager->selectedItem.head;
    while (temp != NULL) {
        Item* existingItem = (Item*)temp->data;
        if (_isItemEqual(existingItem, item)) {
            // Item sudah ada, tidak perlu ditambahkan lagi
            item->selected = true; // Pastikan flag selected konsisten
            return;
        }
        temp = temp->next;
    }

    // Item belum ada, tambahkan ke list
    item->selected = true;

    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("[LOG] Gagal mengalokasi memory untuk selected item\n");
        return;
    }

    newNode->data = item; // Simpan pointer ke item, bukan copy
    newNode->next = fileManager->selectedItem.head;
    fileManager->selectedItem.head = newNode;

    printf("[LOG] Item %s berhasil ditambahkan ke selection\n", item->name);
}

void clearSelectedFile(FileManager* fileManager) {
    Node* current = fileManager->selectedItem.head;

    // First, unmark all items as unselected
    while (current != NULL) {
        Item* item = (Item*)current->data;
        if (item) {
            item->selected = false;
        }
        current = current->next;
    }

    // Then free all nodes
    current = fileManager->selectedItem.head;
    while (current != NULL) {
        Node* next = current->next;
        // _freeSelectedNode(current);
        current = next;
    }

    fileManager->selectedItem.head = NULL;
    printf("[LOG] Semua item berhasil di-deselect\n");
}

void deselectFile(FileManager* fileManager, Item* item) {
    Node* current = fileManager->selectedItem.head;
    Node* prev = NULL;

    while (current != NULL) {
        Item* data = (Item*)current->data;
        if (_isItemEqual(data, item)) {
            // Unmark item
            item->selected = false;

            // Remove from linked list
            if (prev == NULL) {
                fileManager->selectedItem.head = current->next;
            }
            else {
                prev->next = current->next;
            }

            _freeSelectedNode(current);
            printf("[LOG] Item %s berhasil di-deselect\n", item->name);
            return;
        }
        prev = current;
        current = current->next;
    }

    // Item tidak ditemukan dalam list, tapi pastikan flag selected false
    item->selected = false;
}

void selectAll(FileManager* fileManager) {
    if (fileManager->treeCursor == NULL) {
        printf("[LOG] Tidak ada direktori yang dipilih\n");
        return;
    }

    // Clear existing selection first
    clearSelectedFile(fileManager);

    Tree currentNode = fileManager->treeCursor->first_son;
    int selectedCount = 0;

    while (currentNode != NULL) {
        selectFile(fileManager, &currentNode->item);
        selectedCount++;
        currentNode = currentNode->next_brother;
    }

    printf("[LOG] %d file di direktori saat ini telah dipilih\n", selectedCount);
}

// Function baru untuk cleanup selection saat pindah direktori
void _cleanupInvalidSelections(FileManager* fileManager) {
    if (!fileManager->treeCursor) return;

    Node* current = fileManager->selectedItem.head;
    Node* prev = NULL;

    while (current != NULL) {
        Item* item = (Item*)current->data;
        Node* next = current->next;

        // Cek apakah item masih ada di current directory
        bool foundInCurrentDir = false;
        Tree child = fileManager->treeCursor->first_son;

        while (child != NULL) {
            if (_isItemEqual(&child->item, item)) {
                foundInCurrentDir = true;
                break;
            }
            child = child->next_brother;
        }

        // Jika item tidak ditemukan di current directory, hapus dari selection
        if (!foundInCurrentDir) {
            item->selected = false;

            if (prev == NULL) {
                fileManager->selectedItem.head = next;
            }
            else {
                prev->next = next;
            }

            _freeSelectedNode(current);
            printf("[LOG] Removed invalid selection: %s\n", item->name);
        }
        else {
            prev = current;
        }

        current = next;
    }
}

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
    operationToRedo->itemTemp = alloc(Queue);
    create_queue(&(*(operationToRedo->itemTemp)));

    switch (operationToUndo->type) {
    case ACTION_CREATE:
        _undoCreate(fileManager, operationToUndo);
        break;
    case ACTION_DELETE:
        _undoDelete(fileManager, operationToUndo, operationToRedo);
        break;
    case ACTION_UPDATE:
        _undoRename(fileManager, operationToUndo);
        break;
    case ACTION_RECOVER:
        _undoRecover(fileManager, operationToUndo, operationToRedo);
        break;
    case ACTION_PASTE:
        _undoPaste(fileManager, operationToUndo, operationToRedo);
        break;
    default:
        printf("[LOG] Unknown operation type: %d\n", operationToUndo->type);
        break;
    }
    push(&(fileManager->redo), operationToRedo);
    // printTree(fileManager->root, 0);
    refreshFileManager(fileManager);
    // printTree(fileManager->root, 0);
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
    operationToUndo->itemTemp = alloc(Queue);
    create_queue(&(*(operationToUndo->itemTemp)));
    switch (operationToRedo->type) {
    case ACTION_CREATE:
        _redoCreate(fileManager, operationToRedo);
        break;
    case ACTION_DELETE:
        _redoDelete(fileManager, operationToRedo, operationToUndo);
        break;
    case ACTION_UPDATE:
        _redoRename(fileManager, operationToRedo);
        break;
    case ACTION_PASTE:
        _redoPaste(fileManager, operationToRedo, operationToUndo);
        break;

    case ACTION_RECOVER:
        _redoRecover(fileManager, operationToRedo, operationToUndo);
        break;
    }
    push(&(fileManager->undo), operationToUndo);
    refreshFileManager(fileManager);
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

            // Free semua allocated memory termasuk UID
            free(trashItem->uid);
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

    // Generate unique ID sebagai primary key
    char* uid = _generateUID();

    // Buat TrashItem dengan metadata lengkap
    TrashItem* trashItem = (TrashItem*)malloc(sizeof(TrashItem));

    // Simpan nama asli untuk display
    trashItem->item = itemTree->item;
    trashItem->item.name = strdup(itemTree->item.name);  // Nama asli untuk display

    trashItem->uid = uid;  // Primary key unik
    trashItem->originalPath = strdup(itemTree->item.path);  // Path asal
    trashItem->deletedTime = time(NULL);  // Waktu dihapus (bukan modified time)

    char* srcPath = itemTree->item.path;

    // Gunakan UID untuk nama file fisik di trash (primary key)
    char* fileExtension = strrchr(itemTree->item.name, '.');
    char* trashFileName;

    if (fileExtension && itemTree->item.type == ITEM_FILE) {
        // Untuk file dengan ekstensi: namaAsli_UID.ext
        size_t nameLen = fileExtension - itemTree->item.name;
        char* nameWithoutExt = malloc(nameLen + 1);
        strncpy(nameWithoutExt, itemTree->item.name, nameLen);
        nameWithoutExt[nameLen] = '\0';

        trashFileName = TextFormat("%s_%s%s", nameWithoutExt, uid, fileExtension);
        free(nameWithoutExt);
    }
    else {
        // Untuk folder atau file tanpa ekstensi: namaAsli_UID
        trashFileName = TextFormat("%s_%s", itemTree->item.name, uid);
    }

    char* trashPath = TextFormat("%s/%s", trashDir, trashFileName);
    trashItem->trashPath = strdup(trashPath);

    // Update item properties untuk trash display
    trashItem->item.path = strdup(trashPath);
    trashItem->item.deleted_at = trashItem->deletedTime;  // Set deleted time

    // Move ke trash secara fisik
    if (rename(srcPath, trashPath) != 0) {
        printf("[LOG] Gagal memindahkan %s ke trash\ntrashPath:%s\nsrcPath:%s\n",
            itemTree->item.name, trashPath, srcPath);

        // Cleanup on failure
        free(trashItem->uid);
        free(trashItem->item.name);
        free(trashItem->originalPath);
        free(trashItem->trashPath);
        free(trashItem->item.path);
        free(trashItem);
        return;
    }

    // Tambahkan ke LinkedList trash
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = trashItem;
    newNode->next = fileManager->trash.head;
    fileManager->trash.head = newNode;

    // Hapus dari tree utama
    remove_node(&(fileManager->root), itemTree);

    printf("[LOG] Item '%s' moved to trash with UID: %s (Deleted at: %ld)\n",
        trashItem->item.name, uid, trashItem->deletedTime);

    // Otomatis save ke file setelah perubahan
    saveTrashToFile(fileManager);
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
    if (!tree) return NULL;

    // Calculate total length needed
    size_t totalLen = 0;
    Tree temp = tree;
    while (temp != NULL) {
        totalLen += strlen(temp->item.name) + 1; // +1 for '/' or null terminator
        temp = temp->parent;
    }

    char* path = malloc(totalLen + 1);
    if (!path) return NULL;

    path[0] = '\0';

    // Build path components in reverse
    char** components = malloc(sizeof(char*) * 100); // Assume max depth 100
    int count = 0;

    temp = tree;
    while (temp != NULL && count < 100) {
        components[count++] = temp->item.name;
        temp = temp->parent;
    }

    // Build final path
    for (int i = count - 1; i >= 0; i--) {
        if (i == count - 1) {
            strcpy(path, components[i]);
        }
        else {
            strcat(path, "/");
            strcat(path, components[i]);
        }
    }

    free(components);
    return path;
}
/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void goTo(FileManager* fileManager, Tree tree) {
    if (!fileManager || !tree)
        return;

    // Validasi tree node masih valid
    if (!_isValidTreeNode(fileManager->root, tree)) {
        printf("[LOG] Warning: Tree node tidak valid, mencari ulang...\n");
        // Cari node berdasarkan path
        if (tree->item.path) {
            tree = _findNodeByPath(fileManager->root, tree->item.path);
            if (!tree) {
                printf("[LOG] Error: Tidak dapat menemukan node target\n");
                return;
            }
        }
        else {
            return;
        }
    }

    fileManager->treeCursor = tree;

    // Free old path before setting new one
    if (fileManager->currentPath && strcmp(fileManager->currentPath, "root") != 0) {
        free(fileManager->currentPath);
    }

    char* newPath = getCurrentPath(tree);
    if (newPath) {
        fileManager->currentPath = newPath;
    }

    clearSelectedFile(fileManager);

    refreshFileManager(fileManager);
    fileManager->isSearching = false;

    // Safe string operations with bounds checking
    if (fileManager->ctx && fileManager->ctx->navbar) {
        memset(fileManager->ctx->navbar->textboxSearch, 0, MAX_STRING_LENGTH);

        if (fileManager->isRootTrash) {
            strncpy(fileManager->ctx->navbar->textboxPath, "root", MAX_STRING_LENGTH - 1);
            fileManager->ctx->navbar->textboxPath[MAX_STRING_LENGTH - 1] = '\0';

            // Free newPath since we're overriding with "root"
            if (newPath) {
                free(newPath);
            }
            fileManager->currentPath = strdup("root");
        }
    }

    printf("[LOG] Navigated to: %s\n", fileManager->currentPath ? fileManager->currentPath : "unknown");
}

// Helper function untuk validasi tree node
bool _isValidTreeNode(Tree root, Tree target) {
    if (!root || !target) return false;

    if (root == target) return true;

    // Cek di children
    Tree current = root->first_son;
    while (current) {
        if (_isValidTreeNode(current, target)) return true;
        current = current->next_brother;
    }

    return false;
}/*  Prosedur
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

/*  Function untuk generate unique ID dengan timestamp microsecond
 *  IS: -
 *  FS: return string UID dengan format YYYYMMDD_HHMMSS_microsecond
================================================================================*/
char* _generateUID() {
    struct timeval tv;
    struct tm* tm_info;
    char timestamp[64];
    char* uid;
    time_t current_time;

    // Get current time dengan microsecond precision
    gettimeofday(&tv, NULL);

    // Convert tv_sec ke time_t untuk kompatibilitas
    current_time = (time_t)tv.tv_sec;
    tm_info = localtime(&current_time);

    // Format: YYYYMMDD_HHMMSS_microsecond
    snprintf(timestamp, sizeof(timestamp), "%04d%02d%02d_%02d%02d%02d_%06ld",
        tm_info->tm_year + 1900,
        tm_info->tm_mon + 1,
        tm_info->tm_mday,
        tm_info->tm_hour,
        tm_info->tm_min,
        tm_info->tm_sec,
        tv.tv_usec);

    uid = strdup(timestamp);
    printf("[LOG] Generated UID: %s\n", uid);
    return uid;
}

/*  Prosedur helper untuk remove dari LinkedList trash berdasarkan UID
 *  IS: UID item yang akan dihapus
 *  FS: Item dihapus dari trash list
================================================================================*/
void _removeFromTrashByUID(FileManager* fileManager, char* uid) {
    Node* current = fileManager->trash.head;
    Node* prev = NULL;

    while (current != NULL) {
        TrashItem* trashItem = (TrashItem*)current->data;
        if (strcmp(trashItem->uid, uid) == 0) {
            if (prev == NULL) {
                fileManager->trash.head = current->next;
            }
            else {
                prev->next = current->next;
            }

            // Free semua allocated memory
            free(trashItem->uid);
            free(trashItem->item.name);
            free(trashItem->item.path);
            free(trashItem->originalPath);
            free(trashItem->trashPath);
            free(trashItem);
            free(current);

            printf("[LOG] Trash item dengan UID %s berhasil dihapus\n", uid);
            return;
        }
        prev = current;
        current = current->next;
    }

    printf("[LOG] Trash item dengan UID %s tidak ditemukan\n", uid);
}

/*  Prosedur helper untuk add back to tree dari trash
 *  IS: TrashItem dan path recovery
 *  FS: Item ditambahkan kembali ke tree
================================================================================*/
void _addBackToTreeFromTrash(FileManager* fileManager, TrashItem* trashItem, char* recoverPath) {
    // Cari parent directory untuk recovery
    char* parentDir = _getDirectoryFromPath(recoverPath);

    // Cari parent node di tree
    Tree parentNode = _findNodeByPath(fileManager->root, parentDir);

    if (parentNode != NULL) {
        // Buat item baru dengan data recovery
        Item recoveredItem = createItem(
            _getNameFromPath(recoverPath),      // Nama dari path recovery
            recoverPath,                        // Path recovery
            0,                                  // Size akan diupdate
            trashItem->item.type,              // Tipe asli
            time(NULL),                        // Created time baru
            time(NULL),                        // Modified time baru
            0                                  // Reset deleted time
        );

        insert_node(parentNode, recoveredItem);
        printf("[LOG] Item '%s' berhasil ditambahkan kembali ke tree di %s\n",
            recoveredItem.name, parentDir);
    }
    else {
        printf("[LOG] Gagal menemukan parent directory di tree: %s\n", parentDir);
    }

    free(parentDir);
}

void _undoCreate(FileManager* fileManager, Operation* operationToUndo) {
    Tree foundTree;
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
}

void _undoDelete(FileManager* fileManager, Operation* operationToUndo, Operation* operationToRedo) {
    Tree foundTree;
    TrashItem* trashItem;
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
        _removeFromTrash(fileManager, trashItem->item.name);
        _deletePermanently(trashItem->trashPath, trashItem->item.type, trashItem->item.name);
        printf("[LOG] Undo delete item: %s\n", trashItem->item.name);
        // enqueue(&(operationToRedo->itemTemp), trashItem);
    }
    printf("[LOG] Undo delete selesai\n");
}

void _undoRecover(FileManager* fileManager, Operation* operationToUndo, Operation* operationToRedo) {
    Tree foundTree;
    TrashItem* trashItem;
    // kembalikan semua item yang sudah di-recover ke trash
    while (!is_queue_empty(*(operationToUndo->itemTemp))) {
        trashItem = (TrashItem*)dequeue(&(*operationToUndo->itemTemp));
        enqueue(&(*operationToRedo->itemTemp), trashItem);
        foundTree = searchTree(fileManager->root, createItem(trashItem->item.name, trashItem->item.path, 0, trashItem->item.type, 0, 0, 0));
        printf("[LOG] Undo recover item: %s\n", trashItem->item.name);
        // Pindahkan kembali ke trash
        _moveToTrash(fileManager, foundTree);
    }
}


void _undoRename(FileManager* fileManager, Operation* operationToUndo) {
    Tree foundTree;
    // Kembalikan nama item yang diubah
    foundTree = searchTree(fileManager->root, createItem(_getNameFromPath(operationToUndo->to), operationToUndo->to, 0, ITEM_FILE, 0, 0, 0));
    if (foundTree != NULL) {
        renameFile(fileManager, foundTree->item.path, _getNameFromPath(operationToUndo->from), false);
        printf("[LOG] Undo update: %s to %s\n", operationToUndo->to, operationToUndo->from);
    }
}

void _undoPaste(FileManager* fileManager, Operation* operationToUndo, Operation* operationToRedo) {
    Tree foundTree;
    operationToRedo->isCopy = operationToUndo->isCopy;
    if (!operationToUndo->isCopy) {
        // Jika cut, perlu mengembalikan item ke lokasi asalnya
        while (!is_queue_empty(*(operationToUndo->itemTemp))) {
            PasteItem* pasteItem = (PasteItem*)dequeue(&(*operationToUndo->itemTemp));
            Tree parentOriginTree = searchTree(fileManager->root, createItem(_getNameFromPath(_getDirectoryFromPath(pasteItem->originalPath)), _getDirectoryFromPath(pasteItem->originalPath), 0, ITEM_FOLDER, 0, 0, 0));
            if (parentOriginTree == NULL) {
                printf("[LOG] Parent origin tree tidak ditemukan untuk path: %s\n", pasteItem->originalPath);
                continue;
            }
            foundTree = searchTree(fileManager->root, createItem(pasteItem->item.name, pasteItem->item.path, 0, pasteItem->item.type, 0, 0, 0));
            if (foundTree != NULL) {
                printf("[LOG] Undo cut path: %s\n", pasteItem->item.path);
                printf("[LOG] Undo cut name: %s\n", pasteItem->item.name);
                // Pindahkan kembali ke lokasi asal
                char* destPath = strdup(pasteItem->item.path);
                char* originalPath = pasteItem->originalPath;
                // Hapus item dari tree
                if (rename(foundTree->item.path, originalPath) != 0) {
                    printf("[LOG] Gagal mengembalikan item %s ke %s\n", pasteItem->item.name, originalPath);
                }
                else {
                    // Update item path di tree
                    foundTree->item.path = strdup(pasteItem->originalPath);
                    foundTree->item.name = strdup(pasteItem->item.name);
                    // Tambahkan ke tree parent asal
                    Item newItem = createItem(foundTree->item.name, foundTree->item.path, foundTree->item.size, foundTree->item.type, foundTree->item.created_at, time(NULL), 0);
                    Tree newTree = insert_node(parentOriginTree, newItem);
                    if (newItem.type == ITEM_FOLDER) {
                        _addTreeStructureRecursive(newTree, newItem.path);
                        printf("[LOG] Folder %s berhasil dikembalikan ke %s\n", pasteItem->item.name, parentOriginTree->item.path);
                    }
                    printf("[LOG] Item %s berhasil dikembalikan ke %s\n", pasteItem->item.name, foundTree->item.path);

                    pasteItem->originalPath = strdup(destPath); // Update originalPath untuk redo
                    pasteItem->item.path = strdup(foundTree->item.path); // Update path item
                    pasteItem->item.name = strdup(foundTree->item.name); // Update name item
                    printf("[LOG] Original path diupdate menjadi: %s\n", pasteItem->originalPath);
                    free(destPath);
                    remove_node(&(fileManager->root), foundTree);
                    printf("[LOG] Item %s berhasil dihapus dari tree\n", pasteItem->item.name);
                }
            }
            else {
                printf("[LOG] Item tidak ditemukan untuk di-undo cut: %s\n", pasteItem->item.name);
            }
            enqueue(&(*operationToRedo->itemTemp), pasteItem);
        }
    }
    else {
        // Jika hanya copy, Hapus item yang sudah di-paste
        while (!is_queue_empty(*(operationToUndo->itemTemp))) {
            PasteItem* pasteItem = (PasteItem*)dequeue(&(*operationToUndo->itemTemp));
            foundTree = searchTree(fileManager->root, createItem(pasteItem->item.name, pasteItem->item.path, 0, pasteItem->item.type, 0, 0, 0));
            if (foundTree != NULL) {
                printf("[LOG] Undo paste path: %s\n", pasteItem->item.path);
                printf("[LOG] Undo paste name: %s\n", pasteItem->item.name);
                printf("[LOG] Original path: %s\n", pasteItem->originalPath);
                char* destPath = strdup(pasteItem->item.path);
                _deleteSingleItem(foundTree->item.path, foundTree->item.type, foundTree->item.name);
                // Hapus dari tree
                remove_node(&(fileManager->root), foundTree);
                // ubah paste item ke original path
                pasteItem->item.path = strdup(pasteItem->originalPath); // Update path item ke original path
                pasteItem->item.name = strdup(_getNameFromPath(pasteItem->originalPath)); // Update name item ke nama asli
                pasteItem->originalPath = strdup(destPath); // Update originalPath untuk redo
                printf("[LOG] Item %s berhasil dihapus dari tree\n", pasteItem->item.name);
                enqueue(&(*operationToRedo->itemTemp), pasteItem);
                printf("[DEBUG] Enqueuing item for redo: name=%s, path=%s, type=%d\n",
                    pasteItem->item.name, pasteItem->item.path, pasteItem->item.type);
            }
            else {
                printf("[LOG] Item tidak ditemukan untuk dihapus: %s\n", pasteItem->item.path);
            }
        }
    }
    printf("[LOG] Undo paste operation completed\n");
}


void _redoCreate(FileManager* fileManager, Operation* operationToRedo) {
    // Buat item yang sudah dihapus
    createFile(fileManager, operationToRedo->isDir ? ITEM_FOLDER : ITEM_FILE, _getDirectoryFromPath(operationToRedo->from), _getNameFromPath(operationToRedo->from), false);
    printf("[LOG] Redo create: %s\n", operationToRedo->from);
}

void _redoDelete(FileManager* fileManager, Operation* operationToRedo, Operation* operationToUndo) {
    Tree foundTree;
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
}

void _redoRename(FileManager* fileManager, Operation* operationToRedo) {
    Tree foundTree;
    foundTree = searchTree(fileManager->root, createItem(_getNameFromPath(operationToRedo->from), operationToRedo->from, 0, ITEM_FILE, 0, 0, 0));
    if (foundTree == NULL) {
        printf("[LOG] Item tidak ditemukan untuk di-rename: %s\n", operationToRedo->from);
        return;
    }
    renameFile(fileManager, foundTree->item.path, _getNameFromPath(operationToRedo->to), false);
    printf("[LOG] Redo rename: %s\n", operationToRedo->from);
}

void _redoPaste(FileManager* fileManager, Operation* operationToRedo, Operation* operationToUndo) {
    Tree foundTree;

    printf("[LOG] isCopy: %s\n", operationToRedo->isCopy ? "true" : "false");
    operationToUndo->isCopy = operationToRedo->isCopy;
    if (!operationToRedo->isCopy) {
        // Jika cut, perlu memindahkan item ke lokasi baru
        while (!is_queue_empty(*(operationToRedo->itemTemp))) {
            PasteItem* pasteItem = (PasteItem*)dequeue(&(*operationToRedo->itemTemp));
            Tree parentToSearch = searchTree(fileManager->root, createItem(_getNameFromPath(_getDirectoryFromPath(pasteItem->originalPath)), _getDirectoryFromPath(pasteItem->originalPath), 0, ITEM_FOLDER, 0, 0, 0));
            if (parentToSearch == NULL) {
                printf("[LOG] Parent directory tidak ditemukan untuk path: %s\n", pasteItem->originalPath);
                continue;
            }
            foundTree = searchTree(fileManager->root, createItem(pasteItem->item.name, pasteItem->item.path, 0, pasteItem->item.type, 0, 0, 0));
            if (foundTree != NULL) {
                printf("[LOG] Undo cut path: %s\n", pasteItem->item.path);
                printf("[LOG] Undo cut name: %s\n", pasteItem->item.name);
                // Pindahkan kembali ke lokasi asal
                char* destPath = strdup(pasteItem->item.path);
                char* originalPath = pasteItem->originalPath;
                if (rename(foundTree->item.path, originalPath) != 0) {
                    printf("[LOG] Gagal mengembalikan item %s ke %s\n", pasteItem->item.name, originalPath);
                }
                else {
                    // Update item path di tree
                    foundTree->item.path = strdup(pasteItem->originalPath);
                    foundTree->item.name = strdup(pasteItem->item.name);
                    // Tambahkan ke tree parent asal
                    Item newItem = createItem(foundTree->item.name, foundTree->item.path, foundTree->item.size, foundTree->item.type, foundTree->item.created_at, time(NULL), 0);
                    Tree newTree = insert_node(parentToSearch, newItem);
                    if (newItem.type == ITEM_FOLDER) {
                        _addTreeStructureRecursive(newTree, newItem.path);
                        printf("[LOG] Folder %s berhasil dikembalikan ke %s\n", pasteItem->item.name, parentToSearch->item.path);
                    }

                    pasteItem->originalPath = strdup(destPath); // Update originalPath untuk redo
                    pasteItem->item.path = strdup(foundTree->item.path); // Update path item
                    pasteItem->item.name = strdup(foundTree->item.name); // Update name item
                    printf("[LOG] Original path diupdate menjadi: %s\n", pasteItem->originalPath);
                    free(destPath);
                    remove_node(&(fileManager->root), foundTree);
                    printf("[LOG] Item %s berhasil dihapus dari tree\n", pasteItem->item.name);
                }
            }
            else {
                printf("[LOG] Item tidak ditemukan untuk di-redo cut: %s\n", pasteItem->item.path);
            }
            enqueue(&(*operationToUndo->itemTemp), pasteItem);
        }
    }
    else {
        // Jika hanya copy, paste ulang
        while (!is_queue_empty(*(operationToRedo->itemTemp))) {
            PasteItem* pasteItem = (PasteItem*)dequeue(&(*operationToRedo->itemTemp));
            printf("[DEBUG] Processing item in redo: name=%s, path=%s, type=%d\n",
                pasteItem->item.name, pasteItem->item.path, pasteItem->item.type);
            Tree parentToSearch = searchTree(fileManager->root, createItem(_getNameFromPath(_getDirectoryFromPath(pasteItem->originalPath)), _getDirectoryFromPath(pasteItem->originalPath), 0, ITEM_FOLDER, 0, 0, 0));
            if (parentToSearch == NULL) {
                printf("[LOG] Parent directory tidak ditemukan untuk path: %s\n", pasteItem->originalPath);
                continue;
            }
            foundTree = searchTree(fileManager->root, createItem(pasteItem->item.name, pasteItem->item.path, 0, pasteItem->item.type, 0, 0, 0));
            if (foundTree != NULL) {
                printf("[LOG] Redo paste path: %s\n", pasteItem->item.path);
                printf("[LOG] Redo paste name: %s\n", pasteItem->item.name);
                printf("[LOG] Destination path: %s\n", pasteItem->originalPath);
                if (pasteItem->item.type == ITEM_FOLDER) {
                    printf("[DEBUG] Redoing paste for folder: %s\n", pasteItem->item.name);
                    if (DirectoryExists(pasteItem->originalPath)) {
                        printf("[LOG] Destination folder already exists: %s\n", pasteItem->originalPath);
                        pasteItem->originalPath = _createDuplicatedFolderName(pasteItem->originalPath, "(1)");
                    }
                    if (MakeDirectory(pasteItem->originalPath) != 0) {
                        printf("[LOG] Gagal membuat folder\n");
                        continue;
                    }
                    printf("[DEBUG] Calling _copyFolderRecursive for folder: %s\n", pasteItem->item.name);
                    _copyFolderRecursive(pasteItem->item.path, pasteItem->originalPath);
                    printf("[DEBUG] Finished _copyFolderRecursive for folder: %s\n", pasteItem->item.name);
                }
                else if (pasteItem->item.type == ITEM_FILE) {
                    printf("[DEBUG] Redoing paste for file: %s\n", pasteItem->item.name);
                    if (FileExists(pasteItem->originalPath)) {
                        printf("[LOG] Destination file already exists: %s\n", pasteItem->originalPath);
                        pasteItem->originalPath = _createDuplicatedFileName(pasteItem->originalPath, "(1)");
                    }

                    _copyFileContent(pasteItem->item.path, pasteItem->originalPath);
                }
                // Tambahkan item baru ke tree di lokasi tujuan
                Item newItem = createItem(
                    pasteItem->item.name,
                    TextFormat("%s/%s", parentToSearch->item.path, pasteItem->item.name), // Update path
                    pasteItem->item.size,
                    pasteItem->item.type,
                    pasteItem->item.created_at,
                    time(NULL),
                    0);
                Tree newTree = insert_node(parentToSearch, newItem);
                if (newTree == NULL) {
                    printf("[LOG] Gagal menambahkan item ke tree selama redo: %s\n", newItem.name);
                }
                else {
                    printf("[LOG] Item berhasil ditambahkan item: %s, ke tree dengan parent: %s\n",
                        newItem.path, parentToSearch->item.path);
                }
                // Update paste item untuk undo
                char* origin = strdup(pasteItem->originalPath);
                pasteItem->originalPath = strdup(pasteItem->item.path); // Update originalPath untuk undo
                pasteItem->item.path = strdup(origin); // Update path item
                pasteItem->item.name = strdup(pasteItem->item.name); // Update name item
                printf("[LOG] Item path diupdate menjadi: %s\n", pasteItem->item.path);
                enqueue(&(*operationToUndo->itemTemp), pasteItem);
                free(origin);
            }
            else {
                printf("[LOG] Item tidak ditemukan untuk di-redo copy: %s\n", pasteItem->item.path);
            }
        }
    }
    printf("[LOG] Redo paste operation completed\n");
    // printTree(fileManager->root, 0);
}

void _redoRecover(FileManager* fileManager, Operation* operationToRedo, Operation* operationToUndo) {

    // 1. Ambil item yang sebelumnya telah dihapus
    while (!is_queue_empty(*(operationToRedo->itemTemp))) {
        TrashItem* trashItem = (TrashItem*)dequeue(&(*operationToRedo->itemTemp));
        enqueue(&(*operationToUndo->itemTemp), trashItem);
        printf("[LOG] Redo recover item: %s\n", trashItem->item.name);
        // 2. Kembalikan item ke tree
        _addBackToTree(fileManager, trashItem, trashItem->originalPath);
    }
}

// Function helper untuk menambah struktur tree secara rekursif
void _addTreeStructureRecursive(Tree parentNode, char* folderPath) {
    DIR* dp;
    struct dirent* ep;
    struct stat statbuf;

    dp = opendir(folderPath);
    if (dp == NULL) {
        printf("[LOG] Gagal membuka direktori: %s\n", folderPath);
        return;
    }

    printf("[LOG] Memproses direktori: %s\n", folderPath);

    while ((ep = readdir(dp)) != NULL) {
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0) {
            continue;
        }

        // PERBAIKAN: Gunakan manual string construction yang aman
        size_t pathLen = strlen(folderPath) + strlen(ep->d_name) + 2;
        char* fullPath = malloc(pathLen);
        if (!fullPath) {
            printf("[LOG] Gagal alokasi memory untuk fullPath\n");
            continue;
        }
        snprintf(fullPath, pathLen, "%s/%s", folderPath, ep->d_name);

        if (stat(fullPath, &statbuf) == 0) {
            ItemType type = S_ISDIR(statbuf.st_mode) ? ITEM_FOLDER : ITEM_FILE;

            // Buat item baru
            Item newItem = createItem(
                strdup(ep->d_name),        // PERBAIKAN: strdup untuk memory safety
                strdup(fullPath),          // PERBAIKAN: strdup untuk memory safety
                statbuf.st_size,
                type,
                statbuf.st_ctime,
                statbuf.st_mtime,
                0
            );

            // Tambahkan ke parent node
            Tree newNode = insert_node(parentNode, newItem);

            printf("[LOG] Added to tree: %s (type: %s, path: %s)\n",
                ep->d_name, type == ITEM_FOLDER ? "FOLDER" : "FILE", fullPath);

            // PERBAIKAN: Jika folder, rekursif tambahkan isinya
            if (type == ITEM_FOLDER && newNode != NULL) {
                printf("[LOG] Recursively processing folder: %s\n", fullPath);
                _addTreeStructureRecursive(newNode, fullPath);
                printf("[LOG] Finished processing folder: %s\n", fullPath);
            }
        }
        else {
            printf("[LOG] Gagal stat file: %s\n", fullPath);
        }

        free(fullPath);
    }

    closedir(dp);
    printf("[LOG] Selesai memproses direktori: %s\n", folderPath);
}

// Tambahkan function helper untuk debug tree structure
void _debugPrintTreeStructure(Tree node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; i++) printf("  ");
    printf("- %s (%s)\n", node->item.name,
        node->item.type == ITEM_FOLDER ? "FOLDER" : "FILE");

    // Print children
    Tree child = node->first_son;
    while (child) {
        _debugPrintTreeStructure(child, depth + 1);
        child = child->next_brother;
    }
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

char* _convertToUnixPath(char* path) {
    if (path == NULL) return NULL;

    // Alokasikan memori untuk path baru
    char* unixPath = malloc(strlen(path) + 1);
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