#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "nbtree.h"
#include "queue.h"
#include "stack.h"

#include "raylib.h"

#include "file_manager.h"
#include "item.h"
#include "operation.h"
#include "utils.h"
#include "win_utils.h"
#include "ctx.h"

#include "gui/component.h"
#include "gui/navbar.h"
#include "gui/sidebar.h"

#define _DIR ".dir/"
#define ROOT ".dir/root"
#define TRASH ".dir/trash"
#define TRASH_DUMP ".dir/.trash"

/*
================================================================================
    PRIVATE HELPER PROTOTYPES
================================================================================
*/

// TRASH OPERATIONS
static void _moveToTrash(FileManager* fm, Tree itemTree);                   // Move item to trash with UID
static void _deletePermanently(char* fullPath, ItemType type, char* name);  // Delete from filesystem
static void _deleteSingleItem(char* fullPath, ItemType type, char* name);   // Delete individual item
static void _removeFromTrash(FileManager* fm, char* itemName);              // Remove from trash list
static void _removeFromTrashByUID(FileManager* fm, char* uid);              // Remove by unique ID

// FILE OPERATIONS
static void _copyFileContent(char* srcPath, char* destPath);                // Copy file data
static void _copyFolderRecursive(char* srcPath, char* destPath);            // Recursively copy folder
static void _loadTree(Tree tree, char* path);                               // Load filesystem into tree
static void _loadTrashFromFile(LinkedList* trash);                          // Load trash from persistent storage
static void _destroyTree(Tree* tree);                                       // Free tree memory recursively

// TREE OPERATIONS
static void _addBackToTree(FileManager* fm, TrashItem* trashItem, char* recoverPath);                               // Restore item to tree
static void remove_node(Tree* root, Tree nodeToRemove);                                                             // Remove node from tree
static void _reconstructTreeStructure(FileManager* fm, Tree sourceTree, char* newBasePath, char* destinationPath);  // Rebuild tree after operation
static void _loadTreeFromPath(Tree parentNode, char* basePath);                                                     // Populate tree from filesystem
static void _addBackToTreeFromTrash(FileManager* fm, TrashItem* trashItem, char* recoverPath);                      // Restore from trash to tree
static void _addTreeStructureRecursive(Tree parentNode, char* folderPath);                                          // Recursively build tree structure

// PATH UTILITIES
static char* _createDuplicatedFolderName(char* dirPath, char* suffix);      // Generate unique folder name
static char* _createDuplicatedFileName(char* filePath, char* suffix);       // Generate unique file name
static bool _isDirectory(char* path);                                       // Check if path is directory
static char* _getDirectoryFromPath(char* path);                             // Extract directory from path
static char* _convertToUnixPath(char* path);                                // Convert Windows to Unix path
static char* _getCurrentPath(Tree tree);                                    // Build path string from tree
static bool _validateImportPath(char* path);                                // Validate import path

// UNDO/REDO OPERATIONS
static void _undoCreate(FileManager* fm, Operation* operationToUndo);                // Undo file creation
static void _undoDelete(FileManager* fm, Operation* opToUndo, Operation* opToRedo);  // Undo deletion
static void _undoRename(FileManager* fm, Operation* op);                             // Undo rename operation
static void _undoPaste(FileManager* fm, Operation* op, Operation* opToRedo);         // Undo paste operation
static void _undoRecover(FileManager* fm, Operation* op, Operation* opToRedo);       // Undo recovery operation
static void _redoCreate(FileManager* fm, Operation* op);                             // Redo file creation
static void _redoDelete(FileManager* fm, Operation* op, Operation* opToUndo);        // Redo deletion
static void _redoRename(FileManager* fm, Operation* op);                             // Redo rename operation
static void _redoPaste(FileManager* fm, Operation* op, Operation* opToUndo);         // Redo paste operation
static void _redoRecover(FileManager* fm, Operation* op, Operation* opToUndo);       // Redo recovery operation

// PASTE OPERATIONS
static int _calculateTotalPasteItems(FileManager* fm);                                                // Count clipboard items
static bool _processSinglePasteItem(FileManager* fm, Item* itemToPaste, char** originPath);           // Validate paste item
static bool _pasteFolderItem(FileManager* fm, Item* itemToPaste, char* originPath, char* newPath);    // Handle folder paste
static bool _pasteFileItem(FileManager* fm, Item* itemToPaste, char* originPath, char* newPath);      // Handle file paste
static void _addItemToCurrentTree(FileManager* fm, Item* itemToPaste, char* newPath, ItemType type);  // Add pasted item to tree
static void _handleCutCleanup(FileManager* fm, Item* itemToPaste);                                    // Clean up after cut operation
static void _createPasteItemRecord(Item* itemToPaste, char* originPath);                              // Create operation record

// SELECTION OPERATIONS
static bool _isItemEqual(Item* item1, Item* item2);                         // Compare items for equality
static void _freeSelectedNode(Node* node);                                  // Free selection node memory
static void _cleanupInvalidSelections(FileManager* fm);                     // Remove invalid selections

// REFRESH OPERATIONS
static void _refreshTreeSafely(FileManager* fm, char* targetPath);          // Safely refresh tree structure
static Tree _findNodeByPath(Tree root, char* targetPath);                   // Find node by path string
static void _refreshSidebarSafely(FileManager* fm);                         // Refresh sidebar with state preservation
static bool _isValidTreeNode(Tree root, Tree target);                       // Validate tree node existence

// SEARCH OPERATIONS
static void _searchingTreeItemRecursive(LinkedList* linkedList, Tree tree, char* keyword);  // Recursive tree search
static void _searchingLinkedListRecursive(FileManager* fm, Node* node, char* keyword);      // Recursive list search
static void _copyToClipboard(FileManager* fm);                                              // Copy selections to clipboard

// UTILITY FUNCTIONS
static char* _generateUID();                                                // Generate unique identifier
static void _debugPrintTreeStructure(Tree node, int depth);                 // Debug tree structure
static bool _shouldShowProgressBar(int totalItems);                         // Determine progress bar necessity
static bool _isExternalPath(char* path);                                    // Check if path is external

/*
================================================================================
--------------------------------------------------------------------------------
                                IMPLEMENTATION
--------------------------------------------------------------------------------
================================================================================
*/

/*
================================================================================
    CORE SYSTEM FUNCTIONS
================================================================================
*/

void createFileManager(FileManager* fm) {
    create_tree(&(fm->root));
    create_list(&(fm->trash));
    create_stack(&(fm->undo));
    create_stack(&(fm->redo));
    create_queue(&(fm->copied));
    create_queue(&(fm->clipboard));
    create_list(&(fm->selectedItem));
    create_list(&(fm->searchingList));
    fm->isRootTrash = false;
    fm->isSearching = false;
    fm->needsRefresh = false;
    fm->isCopy = false;
    fm->currentPath = NULL;
    fm->treeCursor = NULL;
    fm->ctx = NULL;
}

void initFileManager(FileManager* fm) {
    Item rootItem;
    if (fm->root == NULL) {
        rootItem = createItem("root", ROOT, 0, ITEM_FOLDER, 0, 0, 0);
        fm->root = create_node_tree(rootItem);
        fm->currentPath = "root";

        fm->treeCursor = fm->root;

        _loadTree(fm->treeCursor, ROOT);

        _loadTrashFromFile(&(fm->trash));

        printf("\n\n");
    }
}

void refreshFileManager(FileManager* fm) {
    // Refresh sidebar dengan cara yang aman
    if (fm->ctx && fm->ctx->sidebar) {
        printf("[LOG] Refreshing sidebar...\n");
        _refreshSidebarSafely(fm);
        printf("[LOG] Sidebar refreshed successfully\n");
    }
}

void saveTrashToFile(FileManager* fm) {
    FILE* trashFile = fopen(TRASH_DUMP, "w");
    if (trashFile == NULL) {
        perror("[ERROR] Gagal membuka file trash untuk menulis");
        return;
    }

    Node* current = fm->trash.head;
    while (current != NULL) {
        TrashItem* trashItem = (TrashItem*)current->data;
        if (trashItem && trashItem->uid && trashItem->item.name && trashItem->originalPath) {
            // Format: UID,originalName,originalPath,deletedTime
            fprintf(trashFile, "%s,%s,%s,%ld\n",
                trashItem->uid,
                trashItem->item.name, // Nama asli untuk display
                trashItem->originalPath,
                trashItem->deletedTime); // Waktu dihapus
        }
        current = current->next;
    }

    fclose(trashFile);
    printf("[LOG] Trash data saved with UID as primary key and deleted time\n");
}

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
                trashItem->item.name,    // Display nama asli
                trashItem->originalPath, // Path asal
                timeStr,                 // Waktu dihapus
                (trashItem->item.type == ITEM_FOLDER) ? "Folder" : "File");
        }
        current = current->next;
    }
    printf("======================\n\n");
}

/*
================================================================================
    FILE OPERATIONS
================================================================================
*/

void createFile(FileManager* fm, ItemType type, char* dirPath, char* name, bool isOperation) {
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
    if (fm->treeCursor && fm->treeCursor->item.path &&
        strcmp(fm->treeCursor->item.path, currentFullPath) == 0) {
        currentNode = fm->treeCursor;
    } else {
        currentNode = searchTree(fm->root,
            createItem(getNameFromPath(currentFullPath), currentFullPath, 0, ITEM_FOLDER, 0, 0, 0));
    }
    // printf("[LOG] \ncurrent path: %s\n", currentFullPath);
    // printf("[LOG] current name: %s\n", getNameFromPath(currentFullPath));
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
            push(&fm->undo, newOperation);
        }
        newItem = createItem(getNameFromPath(path), path, 0, type, createdTime, createdTime, -1);
        insert_node(currentNode, newItem);

        refreshFileManager(fm);
    }
    else {
        printf("[LOG] Direktori parent tidak ditemukan : %s\n", dirPath);
    }
}

void deleteFile(FileManager* fm, bool isOperation) {
    Operation* deleteOperation;
    if (fm->selectedItem.head == NULL) {
        printf("[LOG] Tidak ada file yang dipilih untuk dihapus\n");
        return;
    }

    Node* temp = fm->selectedItem.head;
    if (isOperation) {
        deleteOperation = alloc(Operation);
        *deleteOperation = createOperation(NULL, NULL, ACTION_DELETE, false, NULL);
        deleteOperation->itemTemp = alloc(Queue);
        create_queue(&(*(deleteOperation->itemTemp)));
    }
    while (temp != NULL) {
        Item* itemToDelete = (Item*)temp->data;
        Tree foundTree = searchTree(fm->root, *itemToDelete);
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

        _moveToTrash(fm, foundTree);
        temp = temp->next;
    }

    push(&fm->undo, deleteOperation);
    clearSelectedFile(fm);
    printf("[LOG] File berhasil dipindah ke trash\n");
}

void deletePermanentFile(FileManager* fm) {
    if (fm->selectedItem.head == NULL) {
        printf("[LOG] Tidak ada file yang dipilih untuk dihapus secara permanen\n");
        return;
    }

    Node* temp = fm->selectedItem.head;
    while (temp != NULL) {
        Item* itemToDelete = (Item*)temp->data;

        Node* trashNode = fm->trash.head;
        TrashItem* foundTrashItem = NULL;

        while (trashNode != NULL) {
            TrashItem* trashItem = (TrashItem*)trashNode->data;
            if (strcmp(trashItem->item.name, itemToDelete->name) == 0 &&
                strcmp(trashItem->item.path, itemToDelete->path) == 0) {
                foundTrashItem = trashItem;
                break;
            }
            trashNode = trashNode->next;
        }

        if (foundTrashItem == NULL) {
            printf("[LOG] Item %s tidak ditemukan di trash, dilewati\n", itemToDelete->name);
            temp = temp->next;
            continue;
        }

        if (foundTrashItem->item.type == ITEM_FILE) {
            if (remove(foundTrashItem->item.path) != 0) {
                printf("[LOG] Gagal menghapus file %s (Error: %d)\n", foundTrashItem->item.name, errno);
                temp = temp->next;
                continue;
            }
        }
        else if (foundTrashItem->item.type == ITEM_FOLDER) {
            char* fullPath = _getFullWindowsPath(foundTrashItem->item.path);
            if (fullPath == NULL) {
                printf("[LOG] Gagal mendapatkan full path folder %s\n", foundTrashItem->item.name);
                temp = temp->next;
                continue;
            }

            if (RemoveItemsRecurse(fullPath) != 1) {
                printf("[LOG] Gagal menghapus folder %s\n", foundTrashItem->item.name);
                free(fullPath);
                temp = temp->next;
                continue;
            }
            free(fullPath);
        }

        _removeFromTrashByUID(fm, foundTrashItem->uid);

        temp = temp->next;
    }

    clearSelectedFile(fm);
    printf("[LOG] File berhasil dihapus secara permanen\n");
}

void renameFile(FileManager* fm, char* filePath, char* newName, bool isOperation) {
    Item item;
    Tree foundTree;
    char* newPath;
    Operation* operationToUndo;
    if (isOperation) {
        operationToUndo = alloc(Operation);
    }
    // Cari item
    item = createItem(getNameFromPath(filePath), filePath, 0, 0, 0, 0, 0);
    foundTree = searchTree(fm->treeCursor, item);
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
        push(&fm->undo, operationToUndo);
    }
    rename(filePath, newPath);

    // update item
    foundTree->item.name = strdup(newName);
    foundTree->item.path = strdup(newPath);

    printf("[LOG] File berhasil diubah namanya menjadi %s\n", newName);
}

void recoverFile(FileManager* fm) {
    if (fm->selectedItem.head == NULL) {
        printf("[LOG] Tidak ada file yang dipilih untuk di-recover\n");
        return;
    }

    Operation* recoverOperation = alloc(Operation);
    *recoverOperation = createOperation(NULL, NULL, ACTION_RECOVER, false, NULL);
    recoverOperation->itemTemp = alloc(Queue);
    create_queue(&(*(recoverOperation->itemTemp)));
    Node* temp = fm->selectedItem.head;
    while (temp != NULL) {
        Item* itemToRecover = (Item*)temp->data;

        // Cari di LinkedList trash berdasarkan UID atau nama display
        Node* trashNode = fm->trash.head;
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
            _removeFromTrashByUID(fm, foundTrashItem->uid);

            // Tambah kembali ke tree dengan path recovery
            _addBackToTreeFromTrash(fm, foundTrashItem, recoverPath);
        }
        else {
            printf("[LOG] Gagal me-recover file %s (UID: %s)\n",
                foundTrashItem->item.name, foundTrashItem->uid);
        }

        temp = temp->next;
    }

    clearSelectedFile(fm);

    // Save perubahan trash
    saveTrashToFile(fm);
}

Item searchFile(FileManager* fm, char* path) {
    Item item = { 0 };
    Item itemToSearch;
    Tree foundTree;

    itemToSearch = createItem(getNameFromPath(path), path, 0, ITEM_FILE, 0, 0, 0);

    foundTree = searchTree(fm->root, itemToSearch);

    if (foundTree == NULL) {
        printf("[LOG] File tidak ditemukan\n");
        return item;
    }
    item = foundTree->item;
    return item;
}

void importFile(FileManager* fm, char* sourcePath, bool isOperation) {
    sourcePath = _convertToUnixPath(sourcePath);
    if (!_validateImportPath(sourcePath)) {
        printf("[LOG] Path tidak valid untuk import: %s\n", sourcePath);
        return;
    }

    if (fm->treeCursor == NULL) {
        printf("[LOG] Tidak ada direktori tujuan yang aktif\n");
        return;
    }

    printf("[LOG] Memulai import dari: %s\n", sourcePath);
    printf("[LOG] Ke direktori: %s\n", fm->treeCursor->item.path);

    // Dapatkan nama file/folder dari path
    char* fileName = getNameFromPath(sourcePath);
    if (fileName == NULL || strlen(fileName) == 0) {
        printf("[LOG] Tidak dapat mengekstrak nama file dari path\n");
        return;
    }

    // Buat path tujuan di direktori saat ini
    char* destinationPath = TextFormat("%s/%s", fm->treeCursor->item.path, fileName);

    // Handle duplikasi nama dengan suffix yang lebih deskriptif
    if (FileExists(destinationPath) || DirectoryExists(destinationPath)) {
        char* timestamp = TextFormat("_imported_%ld", time(NULL) % 10000);
        if (_isDirectory(sourcePath)) {
            destinationPath = _createDuplicatedFolderName(destinationPath, timestamp);
        }
        else {
            destinationPath = _createDuplicatedFileName(destinationPath, timestamp);
        }
        printf("[LOG] File exists, using new name: %s\n", getNameFromPath(destinationPath));
    }

    // Tentukan tipe item dan size untuk progress
    ItemType itemType = _isDirectory(sourcePath) ? ITEM_FOLDER : ITEM_FILE;
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
                getNameFromPath(destinationPath),
                destinationPath,
                statbuf.st_size,
                itemType,
                statbuf.st_ctime,
                time(NULL),
                0);

            Tree newNode = insert_node(fm->treeCursor, newItem);

            // Jika folder, load strukturnya
            if (itemType == ITEM_FOLDER && newNode != NULL) {
                printf("[LOG] Loading tree structure untuk folder...\n");
                _loadTree(newNode, destinationPath);
            }

            // Simpan operasi untuk undo jika diminta
            if (isOperation) {
                Operation* importOperation = alloc(Operation);
                *importOperation = createOperation(destinationPath, sourcePath, ACTION_CREATE, (itemType == ITEM_FOLDER), NULL);
                // push(&fm->undo, importOperation);
                printf("[LOG] Operasi import disimpan untuk undo\n");
            }

            // Refresh file manager untuk update UI
            refreshFileManager(fm);

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

/*
================================================================================
    CLIPBOARD OPERATIONS
================================================================================
*/

void copyFile(FileManager* fm) {
    _copyToClipboard(fm);
    fm->isCopy = true;
    printf("[LOG] Mode: COPY\n");
}

void cutFile(FileManager* fm) {
    _copyToClipboard(fm);
    fm->isCopy = false;
    printf("[LOG] Mode: CUT\n");
}

void pasteFile(FileManager* fm, bool isOperation) {
    if (is_queue_empty(fm->clipboard)) {
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
    int totalItems = _calculateTotalPasteItems(fm);
    bool showProgress = _shouldShowProgressBar(totalItems);
    int currentProgress = 0;
    bool cancelled = false;

    printf("[LOG] Total items to paste: %d, Show progress: %s\n",
        totalItems, showProgress ? "true" : "false");

    // PERBAIKAN: Buat temporary queue untuk iterasi
    Queue tempQueue;
    create_queue(&tempQueue);
    printf("[LOG] Temporary queue created for paste operation\n");
    // Copy semua items ke temporary queue
    Node* temp = fm->clipboard.front;

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
        if (!_processSinglePasteItem(fm, itemToPaste, &originPath)) {
            currentProgress++;
            continue;
        }

        // Path untuk file/folder baru di lokasi tujuan
        char* destinationFullPath = TextFormat("%s%s", _DIR, fm->currentPath);
        char* newPath = TextFormat("%s/%s", destinationFullPath, itemToPaste->name);

        bool success = false;
        // Proses berdasarkan tipe item
        if (isOperation) {
            pasteOperation->isCopy = fm->isCopy;
            printf("[LOG] Paste operation type: %s\n", fm->isCopy ? "COPY" : "CUT");
        }
        if (itemToPaste->type == ITEM_FOLDER) {
            success = _pasteFolderItem(fm, itemToPaste, originPath, newPath);
        }
        else if (itemToPaste->type == ITEM_FILE) {
            success = _pasteFileItem(fm, itemToPaste, originPath, newPath);
        }

        // Di dalam pasteFile function, setelah success = true
        if (success) {
            _handleCutCleanup(fm, itemToPaste);

            // PERBAIKAN: Pastikan destinationFullPath benar
            Tree parentDestinationTree = searchTree(fm->root,
                createItem(getNameFromPath(destinationFullPath), destinationFullPath, 0, ITEM_FOLDER, 0, 0, 0));

            if (parentDestinationTree != NULL) {
                // Tambahkan item utama
                Item newItem = createItem(
                    strdup(getNameFromPath(newPath)), // PERBAIKAN: strdup untuk safety
                    strdup(newPath),                   // PERBAIKAN: strdup untuk safety
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
                    getNameFromPath(destinationFullPath), destinationFullPath);
            }
        }
        currentProgress++;
    }

    // Cleanup temporary queue
    clear_queue(&tempQueue);

    // Reset progress state setelah selesai
    resetProgressBarState();

    // Clear clipboard setelah cut operation (hanya jika tidak di-cancel)
    if (!fm->isCopy && !cancelled) {
        clear_queue(&fm->clipboard);
        printf("[LOG] Clipboard cleared after cut operation\n");
    }

    if (cancelled) {
        printf("[LOG] Paste operation was cancelled\n");
    }
    else {
        printf("[LOG] Paste berhasil!\n");
    }
    if (isOperation) {
        push(&fm->undo, pasteOperation);
    }
    refreshFileManager(fm);
}

/*
================================================================================
    SELECTION MANAGEMENT
================================================================================
*/

void selectFile(FileManager* fm, Item* item) {
    // Cek apakah item sudah ada dalam list
    Node* temp = fm->selectedItem.head;
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
    newNode->next = fm->selectedItem.head;
    fm->selectedItem.head = newNode;

    printf("[LOG] Item %s berhasil ditambahkan ke selection\n", item->name);
}

void clearSelectedFile(FileManager* fm) {
    Node* current = fm->selectedItem.head;

    // First, unmark all items as unselected
    while (current != NULL) {
        Item* item = (Item*)current->data;
        if (item) {
            item->selected = false;
        }
        current = current->next;
    }

    Node* temp = fm->trash.head;
    while (temp != NULL) {
        TrashItem* trashItem = (TrashItem*)temp->data;
        if (trashItem) {
            trashItem->item.selected = false; // Unmark item in trash
        }
        temp = temp->next;
    }

    // Then free all nodes
    current = fm->selectedItem.head;
    while (current != NULL) {
        Node* next = current->next;
        // _freeSelectedNode(current);
        current = next;
    }

    fm->selectedItem.head = NULL;
    printf("[LOG] Semua item berhasil di-deselect\n");
}

void deselectFile(FileManager* fm, Item* item) {
    Node* current = fm->selectedItem.head;
    Node* prev = NULL;

    while (current != NULL) {
        Item* data = (Item*)current->data;
        if (_isItemEqual(data, item)) {
            // Unmark item
            item->selected = false;

            // Remove from linked list
            if (prev == NULL) {
                fm->selectedItem.head = current->next;
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

void selectAll(FileManager* fm) {
    if (fm->treeCursor == NULL) {
        printf("[LOG] Tidak ada direktori yang dipilih\n");
        return;
    }

    // Clear existing selection first
    clearSelectedFile(fm);

    Tree currentNode = fm->treeCursor->first_son;
    int selectedCount = 0;

    while (currentNode != NULL) {
        selectFile(fm, &currentNode->item);
        selectedCount++;
        currentNode = currentNode->next_brother;
    }

    printf("[LOG] %d file di direktori saat ini telah dipilih\n", selectedCount);
}

/*
================================================================================
    NAVIGATION AND SEARCH
================================================================================
*/

void searchingTreeItem(FileManager* fm, char* keyword) {
    if (fm->searchingList.head != NULL) {
        destroy_list(&(fm->searchingList));
        printf("[LOG] Hasil pencarian sebelumnya telah dihapus\n");
    }

    if (keyword == NULL || strlen(keyword) == 0) {
        printf("[LOG] Keyword pencarian tidak boleh kosong\n");
        return;
    }

    _searchingTreeItemRecursive(&(fm->searchingList), fm->treeCursor, keyword);

    if (fm->searchingList.head == NULL) {
        printf("[LOG] Tidak ada file yang cocok dengan keyword '%s'\n", keyword);
    }
    else {
        printf("[LOG] Pencarian selesai. Gunakan printSearchingList() untuk melihat hasil.\n");
    }
}

void searchingLinkedListItem(FileManager* fm, Node* node, char* keyword) {
    if (node == NULL)
        return;

    if (fm->searchingList.head != NULL) {
        destroy_list(&(fm->searchingList));
        printf("[LOG] Hasil pencarian sebelumnya telah dihapus\n");
    }

    if (keyword == NULL || strlen(keyword) == 0) {
        printf("[LOG] Keyword pencarian tidak boleh kosong\n");
        return;
    }

    _searchingLinkedListRecursive(fm, node, keyword);

    if (fm->searchingList.head == NULL) {
        printf("[LOG] Tidak ada item yang cocok dengan keyword '%s'\n", keyword);
    }
    else {
        printf("[LOG] Pencarian selesai. Gunakan printSearchingList() untuk melihat hasil.\n");
    }
}

void printSearchingList(FileManager* fm) {
    if (fm->searchingList.head == NULL) {
        printf("[LOG] Tidak ada hasil pencarian\n");
        return;
    }

    Node* temp = fm->searchingList.head;
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

void goTo(FileManager* fm, Tree tree) {
    if (!fm || !tree)
        return;

    // Validasi tree node masih valid
    if (!_isValidTreeNode(fm->root, tree)) {
        printf("[LOG] Warning: Tree node tidak valid, mencari ulang...\n");
        // Cari node berdasarkan path
        if (tree->item.path) {
            tree = _findNodeByPath(fm->root, tree->item.path);
            if (!tree) {
                printf("[LOG] Error: Tidak dapat menemukan node target\n");
                return;
            }
        }
        else {
            return;
        }
    }

    fm->treeCursor = tree;

    char* temp = fm->currentPath;

    // // Free old path before setting new one
    // if (fm->currentPath && strcmp(fm->currentPath, "root") != 0) {
    //     free(fm->currentPath);
    // }

    char* newPath = _getCurrentPath(tree);
    if (newPath) {
        fm->currentPath = newPath;
    }

    clearSelectedFile(fm);

    refreshFileManager(fm);
    fm->isSearching = false;

    // Safe string operations with bounds checking
    if (fm->ctx && fm->ctx->navbar) {
        memset(fm->ctx->navbar->textboxSearch, 0, MAX_STRING_LENGTH);

        if (fm->isRootTrash) {
            strncpy(fm->ctx->navbar->textboxPath, "root", MAX_STRING_LENGTH - 1);
            fm->ctx->navbar->textboxPath[MAX_STRING_LENGTH - 1] = '\0';

            // Free newPath since we're overriding with "root"
            if (newPath) {
                free(newPath);
            }
            fm->currentPath = strdup("root");
        }
    }

    printf("[LOG] Navigated to: %s\n", fm->currentPath ? fm->currentPath : "unknown");
}

void goBack(FileManager* fm) {
    if (!fm || !fm->treeCursor)
        return;

    Tree parent = fm->treeCursor->parent;
    if (parent)
        goTo(fm, parent);
}

Tree getCurrentRoot(FileManager fm) {
    if (fm.treeCursor == NULL) {
        return NULL;
    }

    Tree currentRoot = fm.treeCursor;
    while (currentRoot->parent != NULL) {
        currentRoot = currentRoot->parent;
    }
    return currentRoot;
}

/*
================================================================================
    UNDO/REDO SYSTEM
================================================================================
*/

void undo(FileManager* fm) {
    Operation* operationToUndo;
    Operation* operationToRedo;
    Tree foundTree;
    TrashItem* trashItem;
    if (fm->undo == NULL) {
        printf("[LOG] No actions to undo.\n");
        return;
    }
    operationToUndo = alloc(Operation);
    operationToRedo = alloc(Operation);
    operationToUndo = (Operation*)pop(&(fm->undo));

    *operationToRedo = createOperation(operationToUndo->from, operationToUndo->to, operationToUndo->type, operationToUndo->isDir, NULL);
    operationToRedo->itemTemp = alloc(Queue);
    create_queue(&(*(operationToRedo->itemTemp)));

    switch (operationToUndo->type) {
    case ACTION_CREATE:
        _undoCreate(fm, operationToUndo);
        break;
    case ACTION_DELETE:
        _undoDelete(fm, operationToUndo, operationToRedo);
        break;
    case ACTION_UPDATE:
        _undoRename(fm, operationToUndo);
        break;
    case ACTION_RECOVER:
        _undoRecover(fm, operationToUndo, operationToRedo);
        break;
    case ACTION_PASTE:
        _undoPaste(fm, operationToUndo, operationToRedo);
        break;
    default:
        printf("[LOG] Unknown operation type: %d\n", operationToUndo->type);
        break;
    }
    push(&(fm->redo), operationToRedo);
    // printTree(fm->root, 0);
    refreshFileManager(fm);
    // printTree(fm->root, 0);
    // printf("[LOG] Redo Pushed : %s\n", operationToUndo->from);
}

void redo(FileManager* fm) {
    Operation* operationToRedo;
    Operation* operationToUndo;
    Tree foundTree;
    if (fm->redo == NULL) {
        printf("[LOG] No actions to redo.\n");
        return;
    }
    operationToRedo = alloc(Operation);
    operationToRedo = (Operation*)pop(&(fm->redo));
    operationToUndo = alloc(Operation);
    *operationToUndo = (Operation){
        .from = operationToRedo->from ? strdup(operationToRedo->from) : NULL,
        .isDir = operationToRedo->isDir,
        .to = operationToRedo->to ? strdup(operationToRedo->to) : NULL,
        .type = operationToRedo->type,
        .itemTemp = NULL };
    operationToUndo->itemTemp = alloc(Queue);
    create_queue(&(*(operationToUndo->itemTemp)));
    switch (operationToRedo->type) {
    case ACTION_CREATE:
        _redoCreate(fm, operationToRedo);
        break;
    case ACTION_DELETE:
        _redoDelete(fm, operationToRedo, operationToUndo);
        break;
    case ACTION_UPDATE:
        _redoRename(fm, operationToRedo);
        break;
    case ACTION_PASTE:
        _redoPaste(fm, operationToRedo, operationToUndo);
        break;
    default:
        printf("[LOG] Unknown operation type: %d\n", operationToRedo->type);
        break;
    }
    push(&(fm->undo), operationToUndo);
    refreshFileManager(fm);
}

/*
================================================================================
    UTILITIES
================================================================================
*/

void windowsOpenWith(char* path) {
    if (path == NULL) {
        return;
    }

    printf("[LOG] %s\n", path);

#ifdef _WIN32
    const char* command = "cmd /c start \"\"";
    int length = strlen(command) + strlen(path) + 5;
    char* executeableCommand = malloc(length);
    if (!executeableCommand) {
        return;
    }

    snprintf(executeableCommand, length, "%s \"%s\" /OPENAS", command, path);
    printf("[LOG] %s\n", executeableCommand);
    system(executeableCommand);
    free(executeableCommand);
#elif defined(__APPLE__)
    const char* command = "open";
    int length = strlen(command) + strlen(path) + 4;
    char* executeableCommand = malloc(length);
    if (!executeableCommand) {
        return;
    }

    snprintf(executeableCommand, length, "%s \"%s\"", command, path);
    printf("[LOG] %s\n", executeableCommand);
    system(executeableCommand);
    free(executeableCommand);
#else
    const char* command = "xdg-open";
    int length = strlen(command) + strlen(path) + 4;
    char* executeableCommand = malloc(length);
    if (!executeableCommand) {
        return;
    }

    snprintf(executeableCommand, length, "%s \"%s\"", command, path);
    printf("[LOG] %s\n", executeableCommand);
    system(executeableCommand);
    free(executeableCommand);
#endif
}

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

char* getNameFromPath(char* path) {
    if (path == NULL) {
        return "";
    }
    char* name = strrchr(path, '/'); // dapatkan string yang dimulai dari karakter slash (/) terakhir
    if (name != NULL) {
        return name + 1; // skip karakter slash (/) terakhir
    }
    return path; // kembalikan pathnya kalau gak ada slash (/) (ini berarti sudah nama file)
};

/*
================================================================================
    PRIVATE HELPER FUNCTIONS - TRASH OPERATIONS
================================================================================
*/

/**
 * @brief Moves an item to trash with unique ID generation
 *
 * Moves a file or folder from the main tree to the trash directory with
 * unique identifier generation for conflict resolution.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] itemTree Tree node representing item to move to trash
 *
 * @pre itemTree is valid node in main tree
 * @post Item moved to trash directory, unique ID generated, item added to trash list
 *
 * @internal
 * @since 1.0
 */
static void _moveToTrash(FileManager* fm, Tree itemTree) {
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
    trashItem->item.name = strdup(itemTree->item.name); // Nama asli untuk display

    trashItem->uid = uid;                                  // Primary key unik
    trashItem->originalPath = strdup(itemTree->item.path); // Path asal
    trashItem->deletedTime = time(NULL);                   // Waktu dihapus (bukan modified time)

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
    trashItem->item.deleted_at = trashItem->deletedTime; // Set deleted time

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
    newNode->next = fm->trash.head;
    fm->trash.head = newNode;

    // Hapus dari tree utama
    remove_node(&(fm->root), itemTree);

    printf("[LOG] Item '%s' moved to trash with UID: %s (Deleted at: %ld)\n",
        trashItem->item.name, uid, trashItem->deletedTime);

    // Otomatis save ke file setelah perubahan
    saveTrashToFile(fm);
}

/**
 * @brief Permanently deletes a file or folder from filesystem
 *
 * Removes file or folder from filesystem without recovery option.
 * Uses appropriate deletion method based on item type.
 *
 * @param[in] fullPath Complete path to item to delete
 * @param[in] type Type of item (file or folder)
 * @param[in] name Display name of item for logging
 *
 * @pre fullPath exists and is accessible
 * @post Item permanently removed from filesystem
 *
 * @internal
 * @since 1.0
 */
static void _deletePermanently(char* fullPath, ItemType type, char* name) {
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

/**
 * @brief Deletes a single item permanently
 *
 * Wrapper function for permanent deletion of individual items.
 * Used primarily for cut operations and undo functionality.
 *
 * @param[in] fullPath Complete path to item
 * @param[in] type Type of item (file or folder)
 * @param[in] name Display name for logging
 *
 * @pre fullPath exists and is accessible
 * @post Item permanently deleted from filesystem
 *
 * @internal
 * @since 1.0
 */
static void _deleteSingleItem(char* fullPath, ItemType type, char* name) {
    // Cut operation = hapus permanen (tidak ke trash)
    _deletePermanently(fullPath, type, name);
}

/**
 * @brief Removes item from trash list by item name
 *
 * Searches trash list and removes item with matching name.
 * Frees all associated memory including paths and metadata.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] itemName Name of item to remove from trash
 *
 * @pre itemName exists in trash list
 * @post Item removed from trash list, memory freed
 *
 * @internal
 * @since 1.0
 */
static void _removeFromTrash(FileManager* fm, char* itemName) {
    Node* current = fm->trash.head;
    Node* prev = NULL;

    while (current != NULL) {
        TrashItem* trashItem = (TrashItem*)current->data;
        if (strcmp(trashItem->item.name, itemName) == 0) {
            if (prev == NULL) {
                fm->trash.head = current->next;
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

/**
 * @brief Removes trash item by unique identifier
 *
 * Searches trash list for item with matching UID and removes it.
 * Frees all associated memory including paths and metadata.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] uid Unique identifier of item to remove
 *
 * @pre uid exists in trash list
 * @post Item with matching UID removed from trash, memory freed
 *
 * @internal
 * @since 1.0
 */
static void _removeFromTrashByUID(FileManager* fm, char* uid) {
    Node* current = fm->trash.head;
    Node* prev = NULL;

    while (current != NULL) {
        TrashItem* trashItem = (TrashItem*)current->data;
        if (strcmp(trashItem->uid, uid) == 0) {
            if (prev == NULL) {
                fm->trash.head = current->next;
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

/*
================================================================================
    PRIVATE HELPER FUNCTIONS - FILE OPERATIONS
================================================================================
*/

/**
 * @brief Copies file content from source to destination
 *
 * Performs binary file copy with buffer-based transfer for efficiency.
 * Handles file opening, reading, writing, and proper resource cleanup.
 *
 * @param[in] srcPath Source file path
 * @param[in] destPath Destination file path
 *
 * @pre srcPath exists and is readable, destPath is writable
 * @post File content copied to destination
 *
 * @internal
 * @since 1.0
 */
static void _copyFileContent(char* srcPath, char* destPath) {
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

/**
 * @brief Recursively copies folder and all contents
 *
 * Performs deep copy of folder structure including all subdirectories
 * and files. Creates destination directory structure as needed.
 *
 * @param[in] srcPath Source folder path
 * @param[in] destPath Destination folder path
 *
 * @pre srcPath exists and is readable, destPath parent exists
 * @post Complete folder structure copied to destination
 *
 * @internal
 * @since 1.0
 */
static void _copyFolderRecursive(char* srcPath, char* destPath) {
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

/**
 * @brief Recursively loads filesystem directory structure into tree
 *
 * Scans specified directory path and populates tree node with all files
 * and subdirectories. Creates complete tree structure mirroring filesystem
 * hierarchy with proper metadata including size, timestamps, and type.
 *
 * @param[in,out] tree Tree node to populate with directory contents
 * @param[in] path Filesystem directory path to scan and load
 *
 * @pre tree is valid allocated tree node, path exists and is accessible
 * @post tree populated with complete directory structure, sizes calculated
 *
 * @note Recursively processes subdirectories to build complete hierarchy
 * @note Calculates cumulative size for parent directories
 * @note Skips "." and ".." directory entries automatically
 * @warning path must be valid directory path with read permissions
 *
 * @see _addTreeStructureRecursive() for similar functionality
 * @see createItem() for item creation details
 *
 * @internal
 * @since 1.0
 */
static void _loadTree(Tree tree, char* path) {
    DIR* dp;
    struct dirent* ep;
    struct stat statbuf;
    long long size = 0;

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

        size += statbuf.st_size;

        if (S_ISDIR(statbuf.st_mode)) {
            Item data = createItem(ep->d_name, fullPath, statbuf.st_size, ITEM_FOLDER, statbuf.st_ctime, statbuf.st_mtime, 0);
            Tree newTree = insert_node(tree, data);
            _loadTree(newTree, fullPath);
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

    tree->item.size = size;

    closedir(dp);
    return;
}

/**
 * @brief Loads persistent trash data from storage file
 *
 * Reads trash dump file and reconstructs trash linked list with all
 * deleted items including metadata. Parses CSV format with UID, original
 * name, original path, and deletion timestamp. Handles memory allocation
 * and proper trash item structure initialization.
 *
 * @param[in,out] trash Pointer to trash linked list to populate
 *
 * @pre trash linked list is properly initialized
 * @post trash list populated with all persistent trash items
 *
 * @note Creates new file if trash dump doesn't exist
 * @note Parses CSV format: UID,originalName,originalPath,deletedTime
 * @note Dynamically allocates memory for line reading
 * @note Reconstructs trash paths based on UID naming convention
 * @warning Memory allocation may fail for large trash files
 *
 * @see saveTrashToFile() for corresponding save operation
 * @see _generateUID() for UID format details
 *
 * @internal
 * @since 1.0
 */
static void _loadTrashFromFile(LinkedList* trash) {
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
                trashItem->item.name = strdup(originalName); // Nama asli untuk display
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

/**
 * @brief Recursively destroys tree structure and frees memory
 *
 * Performs complete cleanup of tree structure by recursively freeing
 * all nodes, allocated strings, and setting tree pointer to NULL.
 * Ensures no memory leaks in tree destruction process.
 *
 * @param[in,out] tree Pointer to tree root to destroy
 *
 * @pre tree pointer is valid (may point to NULL tree)
 * @post All tree memory freed, tree pointer set to NULL
 *
 * @note Recursively processes all children and siblings
 * @note Frees item name and path strings before freeing nodes
 * @note Safe to call with NULL tree pointer
 * @warning Tree becomes unusable after this operation
 *
 * @see createFileManager() for tree initialization
 *
 * @internal
 * @since 1.0
 */
static void _destroyTree(Tree* tree) {
    if (*tree == NULL)
        return;

    _destroyTree(&(*tree)->first_son);
    _destroyTree(&(*tree)->next_brother);

    free((*tree)->item.name);
    free((*tree)->item.path);
    free(*tree);
    *tree = NULL;
}

/*
================================================================================
    PRIVATE HELPER FUNCTIONS - TREE OPERATIONS
================================================================================
*/

/**
 * @brief Adds recovered item back to main tree structure
 *
 * Restores item from trash to main tree at specified recovery path.
 * Updates item metadata and establishes proper tree relationships.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] trashItem Trash item to recover
 * @param[in] recoverPath Path where item should be recovered
 *
 * @pre trashItem is valid, recoverPath parent exists in tree
 * @post Item added back to main tree at recovery location
 *
 * @internal
 * @since 1.0
 */
static void _addBackToTree(FileManager* fm, TrashItem* trashItem, char* recoverPath) {
    // Update item dengan path baru
    Item recoveredItem = trashItem->item;
    recoveredItem.path = strdup(fm->currentPath);
    recoveredItem.name = strdup(getNameFromPath(recoverPath));
    recoveredItem.deleted_at = 0; // Reset deleted time

    // Cari parent node di tree
    Tree parentNode = searchTree(fm->root,
        createItem(getNameFromPath(fm->currentPath),
            fm->currentPath, 0, ITEM_FOLDER, 0, 0, 0));

    if (parentNode != NULL) {
        // Insert ke tree sebagai child dari current directory
        insert_node(parentNode, recoveredItem);
        printf("[LOG] Item berhasil ditambahkan kembali ke tree\n");
    }
    else {
        printf("[LOG] Gagal menemukan parent directory di tree\n");
    }
}

/**
 * @brief Removes node from tree structure
 *
 * Safely removes tree node and updates parent-child relationships.
 * Handles both first child and sibling node removal cases.
 *
 * @param[in,out] root Pointer to root tree node
 * @param[in] nodeToRemove Node to remove from tree
 *
 * @pre nodeToRemove exists in tree structure
 * @post Node removed from tree, relationships updated
 *
 * @internal
 * @since 1.0
 */
static void remove_node(Tree* root, Tree nodeToRemove) {
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

/**
 * @brief Reconstructs tree structure after paste operation
 *
 * Rebuilds tree structure in destination after copy/cut operation.
 * Maintains proper parent-child relationships and metadata.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] sourceTree Source tree structure
 * @param[in] newBasePath New base path for reconstruction
 * @param[in] destinationPath Final destination path
 *
 * @pre sourceTree valid, destination paths accessible
 * @post Tree structure reconstructed at destination
 *
 * @internal
 * @since 1.0
 */
static void _reconstructTreeStructure(FileManager* fm, Tree sourceTree, char* newBasePath, char* destinationPath) {
    // Fix: Use treeCursor directly instead of searching
    Tree parentNode = fm->treeCursor;

    if (parentNode == NULL) {
        printf("[LOG] Parent node tidak ditemukan\n");
        return;
    }

    // Buat node baru untuk folder utama
    Item newFolderItem = createItem(
        getNameFromPath(newBasePath),
        newBasePath,
        0, // size - akan diupdate oleh loadTree
        ITEM_FOLDER,
        time(NULL), // created_at
        time(NULL), // modified_at
        0);

    Tree newFolderNode = insert_node(parentNode, newFolderItem);

    // Rekursif untuk semua child
    // _loadTreeFromPath(newFolderNode, newBasePath);
    _loadTree(newFolderNode, newBasePath);
}

/**
 * @brief Loads tree structure from filesystem path
 *
 * Populates tree node with filesystem contents from specified path.
 * Used for rebuilding tree structure after operations.
 *
 * @param[in,out] parentNode Parent tree node to populate
 * @param[in] basePath Filesystem path to scan
 *
 * @pre parentNode valid, basePath exists and is accessible
 * @post Tree populated with filesystem contents
 *
 * @internal
 * @since 1.0
 */
static void _loadTreeFromPath(Tree parentNode, char* basePath) {
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

/**
 * @brief Adds recovered item back to tree from trash
 *
 * Restores trash item to main tree structure at recovery path.
 * Creates new tree node with recovered item data.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] trashItem Trash item to restore
 * @param[in] recoverPath Path where item should be recovered
 *
 * @pre trashItem is valid, recoverPath parent exists in tree
 * @post Item added back to tree at recovery location
 *
 * @internal
 * @since 1.0
 */
static void _addBackToTreeFromTrash(FileManager* fm, TrashItem* trashItem, char* recoverPath) {
    // Cari parent directory untuk recovery
    char* parentDir = _getDirectoryFromPath(recoverPath);

    // Cari parent node di tree
    Tree parentNode = _findNodeByPath(fm->root, parentDir);

    if (parentNode != NULL) {
        // Buat item baru dengan data recovery
        Item recoveredItem = createItem(
            getNameFromPath(recoverPath), // Nama dari path recovery
            recoverPath,                   // Path recovery
            0,                             // Size akan diupdate
            trashItem->item.type,          // Tipe asli
            time(NULL),                    // Created time baru
            time(NULL),                    // Modified time baru
            0                              // Reset deleted time
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

/**
 * @brief Recursively adds folder structure to tree
 *
 * Scans filesystem folder and adds all contents to tree node recursively.
 * Used for rebuilding tree structure after folder operations.
 *
 * @param[in,out] parentNode Parent tree node to add structure to
 * @param[in] folderPath Filesystem folder path to scan
 *
 * @pre parentNode valid, folderPath exists and is accessible
 * @post Complete folder structure added to tree recursively
 *
 * @internal
 * @since 1.0
 */
static void _addTreeStructureRecursive(Tree parentNode, char* folderPath) {
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
                strdup(ep->d_name), // PERBAIKAN: strdup untuk memory safety
                strdup(fullPath),   // PERBAIKAN: strdup untuk memory safety
                statbuf.st_size,
                type,
                statbuf.st_ctime,
                statbuf.st_mtime,
                0);

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

/*
================================================================================
    PRIVATE HELPER FUNCTIONS - PATH UTILITIES
================================================================================
*/

/**
 * @brief Creates unique folder name to avoid conflicts
 *
 * Generates unique folder name by appending suffix and checking for conflicts.
 * Recursively increments suffix until unique name found.
 *
 * @param[in] dirPath Original directory path
 * @param[in] suffix Suffix to append for uniqueness
 * @return Unique directory path string
 *
 * @pre dirPath and suffix are valid strings
 * @post Unique directory path returned
 *
 * @internal
 * @since 1.0
 */
static char* _createDuplicatedFolderName(char* filePath, char* suffix) {
    char* newPath = TextFormat("%s%s", filePath, suffix);
    if (DirectoryExists(newPath)) {
        newPath = _createDuplicatedFolderName(newPath, suffix);
    }
    return newPath;
}

/**
 * @brief Creates unique filename to avoid conflicts
 *
 * Generates unique filename by inserting suffix before extension.
 * Handles files with and without extensions appropriately.
 *
 * @param[in] filePath Original file path
 * @param[in] suffix Suffix to insert for uniqueness
 * @return Unique file path string
 *
 * @pre filePath and suffix are valid strings
 * @post Unique file path returned
 *
 * @internal
 * @since 1.0
 */
static char* _createDuplicatedFileName(char* filePath, char* suffix) {
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

/**
 * @brief Checks if path represents a directory
 *
 * Uses filesystem stat to determine if path is a directory.
 * Returns boolean result for type checking.
 *
 * @param[in] path Path to check
 * @return true if path is directory, false otherwise
 *
 * @pre path is valid and accessible
 * @post Boolean result indicating directory status
 *
 * @internal
 * @since 1.0
 */
static bool _isDirectory(char* path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

/**
 * @brief Extracts directory portion from file path
 *
 * Returns directory path by finding last separator and truncating.
 * Allocates new string for directory portion.
 *
 * @param[in] path Complete file path
 * @return Directory portion of path as new string
 *
 * @pre path is valid string
 * @post Directory path returned as allocated string
 *
 * @internal
 * @since 1.0
 */
static char* _getDirectoryFromPath(char* path) {
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

/**
 * @brief Converts Windows path to Unix path format
 *
 * Replaces backslash ('\') with forward slash ('/') for cross-platform
 * compatibility. Allocates new string for converted path.
 *
 * @param[in] path Windows-style path string
 * @return Unix-style path string with forward slashes
 *
 * @pre path is valid string or NULL
 * @post Converted
 *
 * @internal
 * @since 1.0
 */
static char* _convertToUnixPath(char* path) {
    if (path == NULL)
        return NULL;

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

/**
 * @brief Constructs current path string from tree hierarchy
 *
 * Traverses tree structure from current node to root to build
 * complete path string with proper separator formatting.
 *
 * @param[in] tree Current tree node to build path from
 * @return Complete path string from root to current node
 *
 * @pre tree is valid tree node
 * @post Path string returned with proper formatting or NULL on failure
 *
 * @internal
 * @since 1.0
 */
static char* _getCurrentPath(Tree tree) {
    if (!tree)
        return NULL;

    // Calculate total length needed
    size_t totalLen = 0;
    Tree temp = tree;
    while (temp != NULL) {
        totalLen += strlen(temp->item.name) + 1; // +1 for '/' or null terminator
        temp = temp->parent;
    }

    char* path = malloc(totalLen + 1);
    if (!path)
        return NULL;

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

/**
 * @brief Validates import path for safety and accessibility
 *
 * Performs comprehensive validation of import path including
 * external path check and Windows path validation.
 *
 * @param[in] path Import path to validate
 * @return true if path is valid for import, false otherwise
 *
 * @pre path is valid string or NULL
 * @post Boolean result indicating import path validity
 *
 * @internal
 * @since 1.0
 */
static bool _validateImportPath(char* path) {
    if (path == NULL || strlen(path) == 0) {
        return false;
    }

    // Cek apakah path eksternal (di luar workspace)
    if (!_isExternalPath(path)) {
        return false;
    }

    // Gunakan Windows utils untuk validasi
    return ValidateWindowsPath(path) == 1;
}

/*
================================================================================
    PRIVATE HELPER FUNCTIONS - UNDO/REDO OPERATIONS
================================================================================
*/

/**
 * @brief Undoes file/folder creation operation
 *
 * Reverses create operation by permanently deleting created item.
 * Used in undo functionality for CREATE operations.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] operationToUndo Operation details for undo
 *
 * @pre operationToUndo contains valid CREATE operation data
 * @post Created item permanently deleted from filesystem and tree
 *
 * @internal
 * @since 1.0
 */
static void _undoCreate(FileManager* fm, Operation* operationToUndo) {
    Tree foundTree;
    printf("[LOG] Undo Path: %s\n", operationToUndo->from);
    // Hapus item yang baru dibuat
    foundTree = searchTree(fm->root, createItem(getNameFromPath(operationToUndo->from), operationToUndo->from, 0, ITEM_FILE, 0, 0, 0));
    if (foundTree != NULL) {
        operationToUndo->isDir = foundTree->item.type == ITEM_FILE ? false : true;
        _deleteSingleItem(foundTree->item.path, foundTree->item.type, foundTree->item.name);
        printf("[LOG] Undo create: %s\n", operationToUndo->from);
    }
    else {
        printf("[LOG] Item tidak ditemukan untuk dihapus: %s\n", operationToUndo->from);
    }
}

/**
 * @brief Undoes delete operation by restoring items
 *
 * Reverses delete operation by restoring items from trash.
 * Recreates items in original locations and removes from trash.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] opToUndo Operation to undo
 * @param[out] opToRedo Operation for redo stack
 *
 * @pre opToUndo contains valid DELETE operation with item queue
 * @post Items restored to original locations, removed from trash
 *
 * @internal
 * @since 1.0
 */
static void _undoDelete(FileManager* fm, Operation* operationToUndo, Operation* operationToRedo) {
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
        createFile(fm, trashItem->item.type, _getDirectoryFromPath(trashItem->originalPath), trashItem->item.name, false);
        _removeFromTrash(fm, trashItem->item.name);
        _deletePermanently(trashItem->trashPath, trashItem->item.type, trashItem->item.name);
        printf("[LOG] Undo delete item: %s\n", trashItem->item.name);
        // enqueue(&(operationToRedo->itemTemp), trashItem);
    }
    printf("[LOG] Undo delete selesai\n");
}

/**
 * @brief Undoes rename operation by restoring original name
 *
 * Reverses rename operation by changing name back to original.
 * Updates both filesystem and tree structure.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] op Operation containing rename details
 *
 * @pre op contains valid RENAME operation with from/to paths
 * @post Item renamed back to original name in filesystem and tree
 *
 * @internal
 * @since 1.0
 */
static void _undoRename(FileManager* fm, Operation* operationToUndo) {
    Tree foundTree;
    // Kembalikan nama item yang diubah
    foundTree = searchTree(fm->root, createItem(getNameFromPath(operationToUndo->to), operationToUndo->to, 0, ITEM_FILE, 0, 0, 0));
    if (foundTree != NULL) {
        renameFile(fm, foundTree->item.path, getNameFromPath(operationToUndo->from), false);
        printf("[LOG] Undo update: %s to %s\n", operationToUndo->to, operationToUndo->from);
    }
}

/**
 * @brief Undoes paste operation based on copy/cut mode
 *
 * Reverses paste operation by either deleting copied items or
 * moving cut items back to original location.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] op Operation to undo
 * @param[out] opToRedo Operation for redo stack
 *
 * @pre op contains valid PASTE operation with item queue
 * @post Paste operation reversed according to copy/cut mode
 *
 * @internal
 * @since 1.0
 */
static void _undoPaste(FileManager* fm, Operation* operationToUndo, Operation* operationToRedo) {
    Tree foundTree;
    operationToRedo->isCopy = operationToUndo->isCopy;
    if (!operationToUndo->isCopy) {
        // Jika cut, perlu mengembalikan item ke lokasi asalnya
        while (!is_queue_empty(*(operationToUndo->itemTemp))) {
            PasteItem* pasteItem = (PasteItem*)dequeue(&(*operationToUndo->itemTemp));
            Tree parentOriginTree = searchTree(fm->root, createItem(getNameFromPath(_getDirectoryFromPath(pasteItem->originalPath)), _getDirectoryFromPath(pasteItem->originalPath), 0, ITEM_FOLDER, 0, 0, 0));
            if (parentOriginTree == NULL) {
                printf("[LOG] Parent origin tree tidak ditemukan untuk path: %s\n", pasteItem->originalPath);
                continue;
            }
            foundTree = searchTree(fm->root, createItem(pasteItem->item.name, pasteItem->item.path, 0, pasteItem->item.type, 0, 0, 0));
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

                    pasteItem->originalPath = strdup(destPath);          // Update originalPath untuk redo
                    pasteItem->item.path = strdup(foundTree->item.path); // Update path item
                    pasteItem->item.name = strdup(foundTree->item.name); // Update name item
                    printf("[LOG] Original path diupdate menjadi: %s\n", pasteItem->originalPath);
                    free(destPath);
                    remove_node(&(fm->root), foundTree);
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
            foundTree = searchTree(fm->root, createItem(pasteItem->item.name, pasteItem->item.path, 0, pasteItem->item.type, 0, 0, 0));
            if (foundTree != NULL) {
                printf("[LOG] Undo paste path: %s\n", pasteItem->item.path);
                printf("[LOG] Undo paste name: %s\n", pasteItem->item.name);
                printf("[LOG] Original path: %s\n", pasteItem->originalPath);
                char* destPath = strdup(pasteItem->item.path);
                _deleteSingleItem(foundTree->item.path, foundTree->item.type, foundTree->item.name);
                // Hapus dari tree
                remove_node(&(fm->root), foundTree);
                // ubah paste item ke original path
                pasteItem->item.path = strdup(pasteItem->originalPath);                   // Update path item ke original path
                pasteItem->item.name = strdup(getNameFromPath(pasteItem->originalPath)); // Update name item ke nama asli
                pasteItem->originalPath = strdup(destPath);                               // Update originalPath untuk redo
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

/**
 * @brief Undoes recover operation by moving items back to trash
 *
 * Reverses recover operation by moving items back to trash.
 * Restores trash state and removes items from main tree.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] op Operation to undo
 * @param[out] opToRedo Operation for redo stack
 *
 * @pre op contains valid RECOVER operation with item queue
 * @post Items moved back to trash, removed from main tree
 *
 * @internal
 * @since 1.0
 */
static void _undoRecover(FileManager* fm, Operation* operationToUndo, Operation* operationToRedo) {
    Tree foundTree;
    TrashItem* trashItem;
    // kembalikan semua item yang sudah di-recover ke trash
    while (!is_queue_empty(*(operationToUndo->itemTemp))) {
        trashItem = (TrashItem*)dequeue(&(*operationToUndo->itemTemp));
        enqueue(&(*operationToRedo->itemTemp), trashItem);
        foundTree = searchTree(fm->root, createItem(trashItem->item.name, trashItem->item.path, 0, trashItem->item.type, 0, 0, 0));
        printf("[LOG] Undo recover item: %s\n", trashItem->item.name);
        // Pindahkan kembali ke trash
        _moveToTrash(fm, foundTree);
    }
}

/**
 * @brief Redoes create operation by recreating item
 *
 * Repeats create operation by recreating previously deleted item.
 * Used in redo functionality after undo of CREATE operation.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] op Operation to redo
 *
 * @pre op contains valid CREATE operation data
 * @post Item recreated in filesystem and tree
 *
 * @internal
 * @since 1.0
 */
static void _redoCreate(FileManager* fm, Operation* operationToRedo) {
    // Buat item yang sudah dihapus
    createFile(fm, operationToRedo->isDir ? ITEM_FOLDER : ITEM_FILE, _getDirectoryFromPath(operationToRedo->from), getNameFromPath(operationToRedo->from), false);
    printf("[LOG] Redo create: %s\n", operationToRedo->from);
}

/**
 * @brief Redoes delete operation by moving items to trash
 *
 * Repeats delete operation by moving items back to trash.
 * Used in redo functionality after undo of DELETE operation.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] op Operation to redo
 * @param[out] opToUndo Operation for undo stack
 *
 * @pre op contains valid DELETE operation with item queue
 * @post Items moved back to trash, removed from main tree
 *
 * @internal
 * @since 1.0
 */
static void _redoDelete(FileManager* fm, Operation* operationToRedo, Operation* operationToUndo) {
    Tree foundTree;
    // kembalikan item ke trash (delete to trash)
    while (!is_queue_empty(*(operationToRedo->itemTemp))) {
        TrashItem* trashItem = (TrashItem*)dequeue(&(*operationToRedo->itemTemp));
        enqueue(&(*operationToUndo->itemTemp), trashItem);
        printf("[LOG] Redo delete item: %s\n", trashItem->item.name);
        foundTree = searchTree(fm->root, createItem(trashItem->item.name, trashItem->item.path, 0, trashItem->item.type, 0, 0, 0));
        if (foundTree == NULL) {
            printf("[LOG] Item tidak ditemukan untuk di-delete kembali: %s\n", trashItem->item.name);
            continue;
        }
        _moveToTrash(fm, foundTree);
        printf("[LOG] Redo delete: %s\n", trashItem->item.name);
    }

    printf("[LOG] Redo delete completed\n");
}

/**
 * @brief Redoes rename operation by applying new name
 *
 * Repeats rename operation by changing name to target name.
 * Used in redo functionality after undo of RENAME operation.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] op Operation to redo
 *
 * @pre op contains valid RENAME operation with from/to paths
 * @post Item renamed to target name in filesystem and tree
 *
 * @internal
 * @since 1.0
 */
static void _redoRename(FileManager* fm, Operation* operationToRedo) {
    Tree foundTree;
    foundTree = searchTree(fm->root, createItem(getNameFromPath(operationToRedo->from), operationToRedo->from, 0, ITEM_FILE, 0, 0, 0));
    if (foundTree == NULL) {
        printf("[LOG] Item tidak ditemukan untuk di-rename: %s\n", operationToRedo->from);
        return;
    }
    renameFile(fm, foundTree->item.path, getNameFromPath(operationToRedo->to), false);
    printf("[LOG] Redo rename: %s\n", operationToRedo->from);
}

/**
 * @brief Redoes paste operation based on copy/cut mode
 *
 * Repeats paste operation by either copying items again or
 * moving items back to destination location.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] op Operation to redo
 * @param[out] opToUndo Operation for undo stack
 *
 * @pre op contains valid PASTE operation with item queue
 * @post Paste operation repeated according to copy/cut mode
 *
 * @internal
 * @since 1.0
 */
static void _redoPaste(FileManager* fm, Operation* operationToRedo, Operation* operationToUndo) {
    Tree foundTree;

    printf("[LOG] isCopy: %s\n", operationToRedo->isCopy ? "true" : "false");
    operationToUndo->isCopy = operationToRedo->isCopy;
    if (!operationToRedo->isCopy) {
        // Jika cut, perlu memindahkan item ke lokasi baru
        while (!is_queue_empty(*(operationToRedo->itemTemp))) {
            PasteItem* pasteItem = (PasteItem*)dequeue(&(*operationToRedo->itemTemp));
            Tree parentToSearch = searchTree(fm->root, createItem(getNameFromPath(_getDirectoryFromPath(pasteItem->originalPath)), _getDirectoryFromPath(pasteItem->originalPath), 0, ITEM_FOLDER, 0, 0, 0));
            if (parentToSearch == NULL) {
                printf("[LOG] Parent directory tidak ditemukan untuk path: %s\n", pasteItem->originalPath);
                continue;
            }
            foundTree = searchTree(fm->root, createItem(pasteItem->item.name, pasteItem->item.path, 0, pasteItem->item.type, 0, 0, 0));
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

                    pasteItem->originalPath = strdup(destPath);          // Update originalPath untuk redo
                    pasteItem->item.path = strdup(foundTree->item.path); // Update path item
                    pasteItem->item.name = strdup(foundTree->item.name); // Update name item
                    printf("[LOG] Original path diupdate menjadi: %s\n", pasteItem->originalPath);
                    free(destPath);
                    remove_node(&(fm->root), foundTree);
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
            Tree parentToSearch = searchTree(fm->root, createItem(getNameFromPath(_getDirectoryFromPath(pasteItem->originalPath)), _getDirectoryFromPath(pasteItem->originalPath), 0, ITEM_FOLDER, 0, 0, 0));
            if (parentToSearch == NULL) {
                printf("[LOG] Parent directory tidak ditemukan untuk path: %s\n", pasteItem->originalPath);
                continue;
            }
            foundTree = searchTree(fm->root, createItem(pasteItem->item.name, pasteItem->item.path, 0, pasteItem->item.type, 0, 0, 0));
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
                pasteItem->item.path = strdup(origin);                  // Update path item
                pasteItem->item.name = strdup(pasteItem->item.name);    // Update name item
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
    // printTree(fm->root, 0);
}

/**
 * @brief Redoes recover operation by restoring items
 *
 * Repeats recover operation by moving items from trash to main tree.
 * Used in redo functionality after undo of RECOVER operation.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] op Operation to redo
 * @param[out] opToUndo Operation for undo stack
 *
 * @pre op contains valid RECOVER operation with item queue
 * @post Items restored from trash to main tree
 *
 * @internal
 * @since 1.0
 */
static void _redoRecover(FileManager* fm, Operation* operationToRedo, Operation* operationToUndo) {

    // 1. Ambil item yang sebelumnya telah dihapus
    while (!is_queue_empty(*(operationToRedo->itemTemp))) {
        TrashItem* trashItem = (TrashItem*)dequeue(&(*operationToRedo->itemTemp));
        enqueue(&(*operationToUndo->itemTemp), trashItem);
        printf("[LOG] Redo recover item: %s\n", trashItem->item.name);
        // 2. Kembalikan item ke tree
        _addBackToTree(fm, trashItem, trashItem->originalPath);
    }
}

/*
================================================================================
    PRIVATE HELPER FUNCTIONS - PASTE OPERATIONS
================================================================================
*/

/**
 * @brief Calculates total number of items in paste operation
 *
 * Counts items in clipboard queue to determine progress bar necessity.
 * Used for UI optimization in bulk paste operations.
 *
 * @param[in] fm Pointer to FileManager instance
 * @return Total number of items in clipboard
 *
 * @pre fm and clipboard are valid
 * @post Item count returned
 *
 * @internal
 * @since 1.0
 */
static int _calculateTotalPasteItems(FileManager* fm) {
    int totalItems = 0;
    Node* countNode = fm->clipboard.front;
    while (countNode != NULL) {
        totalItems++;
        countNode = countNode->next;
    }
    return totalItems;
}

/**
 * @brief Processes single item in paste operation
 *
 * Validates and prepares single item for paste operation.
 * Updates origin path and performs existence checks.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] itemToPaste Item to process for paste
 * @param[out] originPath Pointer to origin path string
 * @return true if item can be pasted, false otherwise
 *
 * @pre itemToPaste is valid item from clipboard
 * @post originPath updated, boolean result indicating paste readiness
 *
 * @internal
 * @since 1.0
 */
static bool _processSinglePasteItem(FileManager* fm, Item* itemToPaste, char** originPath) {
    // Path untuk file/folder baru di lokasi tujuan
    char* destinationFullPath = TextFormat("%s%s", _DIR, fm->currentPath);
    char* newPath = TextFormat("%s/%s", destinationFullPath, itemToPaste->name);
    *originPath = itemToPaste->path;

    printf("[LOG] Mencoba paste: %s -> %s\n", *originPath, newPath);
    printf("[LOG] Is Copy: %s\n", fm->isCopy ? "true" : "false");
    printf("[LOG] Item type: %d (ITEM_FOLDER=%d, ITEM_FILE=%d)\n", itemToPaste->type, ITEM_FOLDER, ITEM_FILE);

    // COPY operation - update origin path
    if (fm->isCopy) {
        Tree foundTree = searchTree(fm->root, *itemToPaste);
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

/**
 * @brief Processes folder item in paste operation
 *
 * Handles folder-specific paste logic including directory creation
 * and recursive copying for copy operations.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] itemToPaste Folder item to paste
 * @param[in] originPath Original path of folder
 * @param[in] newPath Destination path for folder
 * @return true if folder paste successful, false otherwise
 *
 * @pre itemToPaste is valid folder, paths are accessible
 * @post Folder pasted to destination according to copy/cut mode
 *
 * @internal
 * @since 1.0
 */
static bool _pasteFolderItem(FileManager* fm, Item* itemToPaste, char* originPath, char* newPath) {
    printf("[LOG] Processing folder: %s\n", itemToPaste->name);

    if (DirectoryExists(newPath)) {
        printf("[LOG] Destination exists, creating duplicate name\n");
        newPath = _createDuplicatedFolderName(newPath, "(1)");
        printf("[LOG] New path after duplicate check: %s\n", newPath);
    }

    // Untuk CUT, pindah langsung. Untuk COPY, buat folder baru lalu copy
    if (!fm->isCopy) {
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
    // _addItemToCurrentTree(fm, itemToPaste, newPath, ITEM_FOLDER);
    return true;
}

/**
 * @brief Processes file item in paste operation
 *
 * Handles file-specific paste logic including content copying
 * and conflict resolution for duplicate names.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] itemToPaste File item to paste
 * @param[in] originPath Original path of file
 * @param[in] newPath Destination path for file
 * @return true if file paste successful, false otherwise
 *
 * @pre itemToPaste is valid file, paths are accessible
 * @post File pasted to destination according to copy/cut mode
 *
 * @internal
 * @since 1.0
 */
static bool _pasteFileItem(FileManager* fm, Item* itemToPaste, char* originPath, char* newPath) {
    printf("[LOG] Processing file: %s\n", itemToPaste->name);

    if (FileExists(newPath)) {
        newPath = _createDuplicatedFileName(newPath, "(1)");
    }

    if (!fm->isCopy) {
        if (rename(originPath, newPath) != 0) {
            printf("[LOG] Gagal memindahkan file %s\n", itemToPaste->name);
            return false;
        }
    }
    else {
        _copyFileContent(originPath, newPath);
    }

    // Tambahkan item baru ke tree di lokasi tujuan
    // _addItemToCurrentTree(fm, itemToPaste, newPath, itemToPaste->type);
    return true;
}

/**
 * @brief Adds pasted item to current tree structure
 *
 * Inserts newly pasted item into tree at current cursor location.
 * Updates tree relationships and metadata.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] itemToPaste Item that was pasted
 * @param[in] newPath Path where item was pasted
 * @param[in] type Type of pasted item
 *
 * @pre item successfully pasted to filesystem
 * @post Item added to tree structure at current location
 *
 * @internal
 * @since 1.0
 */
static void _addItemToCurrentTree(FileManager* fm, Item* itemToPaste, char* newPath, ItemType type) {
    Tree currentNode = fm->treeCursor;
    if (currentNode != NULL) {
        Item newItem = createItem(
            getNameFromPath(newPath),
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

/**
 * @brief Handles cleanup after cut operation
 *
 * Removes cut items from original location in tree structure.
 * Only applies to cut operations, not copy operations.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] itemToPaste Item that was cut and pasted
 *
 * @pre item was cut (not copied) and successfully pasted
 * @post Item removed from original tree location
 *
 * @internal
 * @since 1.0
 */
static void _handleCutCleanup(FileManager* fm, Item* itemToPaste) {
    if (!fm->isCopy) {
        Tree foundTree = searchTree(fm->root, *itemToPaste);
        if (foundTree != NULL) {
            remove_node(&(fm->root), foundTree);
            printf("[LOG] Item %s berhasil dihapus dari tree asal\n", itemToPaste->name);
        }
    }
}

/**
 * @brief Creates paste item record for operation tracking
 *
 * Creates PasteItem structure for undo/redo operation tracking.
 * Stores original path and item details for operation reversal.
 *
 * @param[in] itemToPaste Item that was pasted
 * @param[in] originPath Original path before paste
 *
 * @pre paste operation completed successfully
 * @post PasteItem record created for operation tracking
 *
 * @internal
 * @since 1.0
 */
static void _createPasteItemRecord(Item* itemToPaste, char* originPath) {
    PasteItem* pasteItem = alloc(PasteItem);
    *pasteItem = createPasteItem(*itemToPaste, originPath);
    printf("[LOG] PasteItem created for %s with original path %s\n", itemToPaste->name, originPath);
}

/*
================================================================================
    PRIVATE HELPER FUNCTIONS - SELECTION OPERATIONS
================================================================================
*/

/**
 * @brief Checks if two items are equal
 *
 * Compares two items by path and name to determine equality.
 * Used for selection management and duplicate detection.
 *
 * @param[in] item1 First item to compare
 * @param[in] item2 Second item to compare
 * @return true if items are equal, false otherwise
 *
 * @pre both items are valid
 * @post Boolean result indicating item equality
 *
 * @internal
 * @since 1.0
 */
static bool _isItemEqual(Item* item1, Item* item2) {
    return (strcmp(item1->path, item2->path) == 0 &&
        strcmp(item1->name, item2->name) == 0);
}

/**
 * @brief Frees selected node and associated memory
 *
 * Safely frees node memory used in selection list.
 * Does not free item data as it points to tree items.
 *
 * @param[in] node Node to free
 *
 * @pre node is valid allocated node
 * @post Node memory freed, item data preserved
 *
 * @internal
 * @since 1.0
 */
static void _freeSelectedNode(Node* node) {
    if (node) {
        // Tidak perlu free item karena itu pointer ke item di tree
        free(node);
    }
}

/**
 * @brief Cleans up invalid selections from selection list
 *
 * Removes items from selection list that are no longer present in
 * the current directory. Validates each selected item against current tree.
 *
 * @param[in,out] fm Pointer to FileManager instance
 *
 * @pre fm and treeCursor are valid
 * @post Invalid selections removed from selectedItem list
 *
 * @internal
 * @since 1.0
 */
static void _cleanupInvalidSelections(FileManager* fm) {
    if (!fm->treeCursor)
        return;

    Node* current = fm->selectedItem.head;
    Node* prev = NULL;

    while (current != NULL) {
        Item* item = (Item*)current->data;
        Node* next = current->next;

        // Cek apakah item masih ada di current directory
        bool foundInCurrentDir = false;
        Tree child = fm->treeCursor->first_son;

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
                fm->selectedItem.head = next;
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

/*
================================================================================
    PRIVATE HELPER FUNCTIONS - REFRESH OPERATIONS
================================================================================
*/

/**
 * @brief Safely refreshes tree structure
 *
 * Refreshes tree at target path while preserving tree integrity.
 * Handles node validation and safe tree reconstruction.
 *
 * @param[in,out] fm Pointer to FileManager instance
 * @param[in] targetPath Path to refresh in tree
 *
 * @pre targetPath exists in tree structure
 * @post Tree refreshed at target path, structure integrity maintained
 *
 * @internal
 * @since 1.0
 */
static void _refreshTreeSafely(FileManager* fm, char* targetPath) {
    // Cari node target di tree
    Tree targetNode = _findNodeByPath(fm->root, targetPath);

    if (!targetNode) {
        printf("[LOG] Target node tidak ditemukan untuk refresh\n");
        return;
    }

    // Hapus hanya children dari target node
    if (targetNode->first_son) {
        _destroyTree(&targetNode->first_son);
        targetNode->first_son = NULL;
    }

    // Load ulang children
    _loadTree(targetNode, targetPath);
}

/**
 * @brief Finds tree node by path
 *
 * Recursively searches tree structure to find node with matching path.
 * Returns pointer to found node or NULL if not found.
 *
 * @param[in] root Root of tree to search
 * @param[in] targetPath Path to search for
 * @return Tree node with matching path or NULL
 *
 * @pre root is valid tree, targetPath is valid string
 * @post Matching node returned or NULL if not found
 *
 * @internal
 * @since 1.0
 */
static Tree _findNodeByPath(Tree root, char* targetPath) {
    if (!root || !targetPath)
        return NULL;

    // Jika path cocok, return node ini
    if (root->item.path && strcmp(root->item.path, targetPath) == 0) {
        return root;
    }

    // Cari di children
    Tree current = root->first_son;
    while (current) {
        Tree found = _findNodeByPath(current, targetPath);
        if (found)
            return found;
        current = current->next_brother;
    }

    return NULL;
}

/**
 * @brief Safely refreshes sidebar component
 *
 * Refreshes sidebar while preserving expansion state and selection.
 * Collects current state before refresh and restores afterward.
 *
 * @param[in,out] fm Pointer to FileManager instance
 *
 * @pre fm has valid sidebar context
 * @post Sidebar refreshed with preserved state
 *
 * @internal
 * @since 1.0
 */
static void _refreshSidebarSafely(FileManager* fm) {
    if (!fm->ctx || !fm->ctx->sidebar)
        return;

    // Collect state sebelum destroy
    SidebarState* stateList = NULL;
    if (fm->ctx->sidebar->sidebarRoot) {
        collectSidebarState(fm->ctx->sidebar->sidebarRoot, &stateList);
        destroySidebarItem(&fm->ctx->sidebar->sidebarRoot);
        fm->ctx->sidebar->sidebarRoot = NULL;
    }

    // Rebuild dengan state yang tersimpan
    fm->ctx->sidebar->sidebarRoot = createSidebarItemWithState(fm->root, stateList);

    // Cleanup state
    if (stateList) {
        destroySidebarState(stateList);
    }
}

/**
 * @brief Validates tree node existence
 *
 * Checks if target node still exists in tree structure.
 * Used for safety validation before tree operations.
 *
 * @param[in] root Root of tree to validate against
 * @param[in] target Target node to validate
 * @return true if target exists in tree, false otherwise
 *
 * @pre root and target are potentially valid tree nodes
 * @post Boolean result indicating node validity
 *
 * @internal
 * @since 1.0
 */
static bool _isValidTreeNode(Tree root, Tree target) {
    if (!root || !target)
        return false;

    if (root == target)
        return true;

    // Cek di children
    Tree current = root->first_son;
    while (current) {
        if (_isValidTreeNode(current, target))
            return true;
        current = current->next_brother;
    }

    return false;
}

/*
================================================================================
    PRIVATE HELPER FUNCTIONS - SEARCH OPERATIONS
================================================================================
*/

/**
 * @brief Recursively searches tree for items matching keyword
 *
 * Performs case-insensitive recursive search through tree structure
 * looking for items whose names contain the specified keyword. Adds
 * matching items to results linked list for later processing.
 *
 * @param[in,out] linkedList Results list to store matching items
 * @param[in] tree Tree node to search from (root for full search)
 * @param[in] keyword Search term to match against item names
 *
 * @pre linkedList is initialized, tree may be NULL, keyword is valid string
 * @post All matching items added to linkedList
 *
 * @note Performs case-insensitive substring matching
 * @note Searches both current node and all descendants recursively
 * @note Processes both children (first_son) and siblings (next_brother)
 * @note Creates lowercase copies for comparison without modifying originals
 * @warning keyword must be valid null-terminated string
 *
 * @see searchingTreeItem() for public interface
 * @see toLowerStr() for case conversion utility
 *
 * @internal
 * @since 1.0
 */
static void _searchingTreeItemRecursive(LinkedList* linkedList, Tree tree, char* keyword) {
    if (tree == NULL)
        return;

    char* lowerName = strdup(tree->item.name);
    char* lowerKeyword = strdup(keyword);

    toLowerStr(lowerName);
    toLowerStr(lowerKeyword);

    if (strstr(lowerName, lowerKeyword) != NULL) {
        printf("===========================================================\n");
        insert_last(linkedList, tree);
        printf("[LOG] keyword: %s\n", keyword);
        printf("[LOG] lower keyword: %s\n", lowerKeyword);
        printf("[LOG] Menemukan item: %s\n", tree->item.name);
        printf("[LOG] Menemukan lower item: %s\n", lowerName);
        printf("[LOG] Path: %s\n", tree->item.path);
        printf("===========================================================\n");
    }

    _searchingTreeItemRecursive(linkedList, tree->first_son, keyword);
    _searchingTreeItemRecursive(linkedList, tree->next_brother, keyword);
}

/**
 * @brief Recursively searches linked list nodes for keyword matches
 *
 * Traverses linked list containing tree node pointers and performs
 * case-insensitive search for items matching keyword. Used for searching
 * within specific result sets or filtered lists.
 *
 * @param[in,out] fm FileManager instance for result storage
 * @param[in] node Current linked list node to process
 * @param[in] keyword Search term to match against item names
 *
 * @pre fm is valid, node may be NULL, keyword is valid string
 * @post Matching items added to fm->searchingList
 *
 * @note Performs case-insensitive substring matching
 * @note Recursively processes remaining list nodes
 * @note Each node->data contains Tree pointer to search
 * @note Creates lowercase copies for safe comparison
 * @warning keyword must be valid null-terminated string
 *
 * @see searchingLinkedListItem() for public interface
 * @see toLowerStr() for case conversion utility
 *
 * @internal
 * @since 1.0
 */
static void _searchingLinkedListRecursive(FileManager* fm, Node* node, char* keyword) {
    if (node == NULL)
        return;

    Tree current = (Tree)node->data;

    char* lowerName = strdup(current->item.name);
    char* lowerKeyword = strdup(keyword);

    toLowerStr(lowerName);
    toLowerStr(lowerKeyword);

    if (strstr(lowerName, lowerKeyword) != NULL) {
        printf("===========================================================\n");
        printf("[LOG] keyword: %s\n", lowerKeyword);
        printf("[LOG] Menemukan item: %s\n [LOG] Path: %s\n", lowerName, current->item.path);
        insert_last(&(fm->searchingList), current);
        printf("===========================================================\n");
    }

    _searchingLinkedListRecursive(fm, node->next, keyword);
}

/**
 * @brief Copies selected items to clipboard for copy/cut operations
 *
 * Transfers all currently selected items to clipboard queue, clearing
 * any previous clipboard contents. Prepares items for subsequent paste
 * operations in copy or cut mode.
 *
 * @param[in,out] fm FileManager instance containing selections and clipboard
 *
 * @pre fm is valid with initialized clipboard and selectedItem list
 * @post Selected items copied to clipboard, previous clipboard cleared
 *
 * @note Clears both clipboard and copied queues before operation
 * @note Copies item pointers, not item data (shallow copy)
 * @note Logs each item added to clipboard for debugging
 * @note Handles empty selection gracefully with error message
 * @warning Requires valid selections in fm->selectedItem list
 *
 * @see copyFile() and cutFile() for public interfaces
 * @see pasteFile() for clipboard consumption
 *
 * @internal
 * @since 1.0
 */
static void _copyToClipboard(FileManager* fm) {
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

/*
================================================================================
    PRIVATE HELPER FUNCTIONS - UTILITY FUNCTIONS
================================================================================
*/

/**
 * @brief Generates unique identifier for trash items
 *
 * Creates timestamp-based UID with microsecond precision for
 * unique identification of deleted items in trash.
 *
 * @return Unique identifier string in YYYYMMDD_HHMMSS_microsecond format
 *
 * @pre System time is available
 * @post Unique UID string returned
 *
 * @internal
 * @since 1.0
 */
static char* _generateUID() {
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

/**
 * @brief Prints tree structure for debugging
 *
 * Recursively prints tree structure with indentation showing hierarchy.
 * Used for debugging tree operations and structure validation.
 *
 * @param[in] node Tree node to print from
 * @param[in] depth Current depth level for indentation
 *
 * @pre node may be valid tree node or NULL
 * @post Tree structure printed to console with hierarchy
 *
 * @internal
 * @since 1.0
 */
static void _debugPrintTreeStructure(Tree node, int depth) {
    if (!node)
        return;

    for (int i = 0; i < depth; i++)
        printf("  ");
    printf("- %s (%s)\n", node->item.name,
        node->item.type == ITEM_FOLDER ? "FOLDER" : "FILE");

    // Print children
    Tree child = node->first_son;
    while (child) {
        _debugPrintTreeStructure(child, depth + 1);
        child = child->next_brother;
    }
}

/**
 * @brief Determines if progress bar should be displayed
 *
 * Evaluates item count to decide whether progress bar is necessary
 * for user feedback during bulk operations.
 *
 * @param[in] totalItems Number of items to be processed
 * @return true if progress bar should be shown, false otherwise
 *
 * @pre totalItems is valid count
 * @post Boolean result indicating progress bar necessity
 *
 * @internal
 * @since 1.0
 */
static bool _shouldShowProgressBar(int totalItems) {
    // Tampilkan progress bar jika item lebih dari 10
    return totalItems > 10;
}

/**
 * @brief Checks if path is external to workspace
 *
 * Determines whether given path is outside the application workspace
 * directory by checking path prefix against workspace markers.
 *
 * @param[in] path File path to validate
 * @return true if path is external to workspace, false otherwise
 *
 * @pre path is valid string or NULL
 * @post Boolean result indicating external path status
 *
 * @internal
 * @since 1.0
 */
static bool _isExternalPath(char* path) {
    if (path == NULL)
        return false;

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
