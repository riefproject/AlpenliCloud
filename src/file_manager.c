#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

#include "file_manager.h"
#include "stack.h"
#include "queue.h"
#include "nbtree.h"
#include "utils.h"
#include "item.h"
#include <time.h>
#include "raylib.h"
#include <string.h>
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
void createFileManager(FileManager *fileManager)
{
    create_tree(&(fileManager->root));
    create_tree(&(fileManager->rootTrash));
    create_stack(&(fileManager->undo));
    create_stack(&(fileManager->redo));
    create_queue(&(fileManager->copied));
    create_queue(&(fileManager->cut));
    create_queue(&(fileManager->temp));
}

void initFileManager(FileManager *fileManager)
{
    Item rootItem;
    if (fileManager->root == NULL)
    {
        rootItem = createItem("root", ROOT, 0, ITEM_FOLDER, 0, 0, 0);
        fileManager->root = create_node_tree(rootItem);
        fileManager->currentPath = ROOT;

        fileManager->treeCursor = fileManager->root;

        loadTree(fileManager->treeCursor, ROOT);
        printf("\n\n");        
        printTree(fileManager->treeCursor, 0);
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

Tree loadTree(Tree tree, char *path)
{
    DIR *dp;
    struct dirent *ep;
    struct stat statbuf;

    dp = opendir(path);
    if (dp == NULL)
    {
        perror("Tidak dapat membuka direktori");
        return -1;
    }

    while ((ep = readdir(dp)) != NULL)
    {
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
            continue;

        char *fullPath = malloc(strlen(path) + strlen(ep->d_name) + 2);
        sprintf(fullPath, "%s/%s", path, ep->d_name);

        if (stat(fullPath, &statbuf) == -1)
        {
            perror("Gagal mendapatkan info file");
            continue;
        }

        time_t now = time(NULL);
        printf("Memproses: %s\n", fullPath);

        if (S_ISDIR(statbuf.st_mode))
        {
            Item data = createItem(ep->d_name, path, (long)statbuf.st_size, ITEM_FOLDER, statbuf.st_ctime, statbuf.st_mtime, 0);
            Tree newTree = insert_node(tree, data);
            printf("  (Direktori) Rekursi ke: %s\n", fullPath);
            loadTree(newTree, fullPath);
        }
        else if (S_ISREG(statbuf.st_mode))
        {
            Item data = createItem(ep->d_name, path, statbuf.st_size, ITEM_FILE, statbuf.st_ctime, statbuf.st_mtime, 0);
            insert_node(tree, data);
            printf("  (File) Ditemukan file: %s\n", fullPath);
        }
        else
        {
            printf("  (Tipe lain) Ditemukan: %s\n", fullPath);
        }

        free(fullPath);
    }

    closedir(dp);
    return 0;
}

void createFile(FileManager *fileManager)
{
    int choice;
    char *fileName = NULL;
    char filepath[512];
    time_t createTime;

    while (true)
    {
        system("cls");
        printf("New Item\n");
        printf("Pilih type:\n");
        printf("1. File\n");
        printf("2. Folder\n");
        printf("Pilih type (1/2): ");
        scanf("%d", &choice);
        getchar();

        if (choice == 1 || choice == 2)
            break;
        printf("Pilihan tidak valid. Tekan enter untuk lanjut...\n");
        getchar();
    }

    printf("Masukkan nama %s: ", (choice == 1) ? "file" : "folder");
    inputString(&fileName);

    snprintf(filepath, sizeof(filepath), "%s/%s", fileManager->currentPath, fileName);

    if (choice == 1)
    {

        FILE *newFile = fopen(filepath, "w");
        if (newFile == NULL)
        {
            perror("Gagal membuat file");
            free(fileName);
            return;
        }
        fclose(newFile);

        // dapatkan waktu saat file dibuat
        createTime = time(NULL);

        Item newNode = createItem(fileName, filepath, 0, ITEM_FILE, createTime, createTime, 0);
        insert_node(fileManager->root, newNode);

        printf("File berhasil dibuat. Tekan enter untuk lanjut...\n");
        getchar();
    }
    else if (choice == 2)
    {
        int status = mkdir(filepath);

        if (status != 0)
        {
            perror("Gagal membuat folder");
            free(fileName);
            return;
        }
        createTime = time(NULL);
        Item item = createItem(fileName, filepath, 0, ITEM_FOLDER, createTime, createTime, 0);
        insert_node(fileManager->root, item);

        printf("Folder berhasil dibuat. Tekan enter untuk lanjut...\n");
        getchar();
    }

    free(fileName);
}

void deleteFile(FileManager *fileManager) {}

// Rename/Update file name
void renameFile(FileManager *fileManager, char *filePath, char *newName)
{
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

void recoverFile(FileManager *fileManager) {}

Item searchFile(FileManager *fileManager, char *path)
{
    Item item = {0};
    Item itemToSearch;
    Tree foundTree;
    itemToSearch = createItem(getNameFromPath(path), path, 0, ITEM_FILE, 0, 0, 0);
    foundTree = searchTree(fileManager->root, item);
    if (foundTree == NULL)
    {
        printf("File tidak ditemukan\n");
        return item;
    }
    item = foundTree->item;
    return item;
}

void undo(FileManager *fileManager) {}
void redo(FileManager *fileManager) {}

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
void copyFile(FileManager *fileManager)
{
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
    fileManager->copied = fileManager->selectedItem;
    isCopy = true;
    if (fileManager->copied.front == NULL)
    {
        printf("Gagal Menyalin File!\n");
        return;
    }
    fileManager->temp = fileManager->copied;
    printf("File berhasil disalin ke clipboard\n");
}

void cutFile(FileManager *fileManager)
{
    if (fileManager->cut.front)
        fileManager->cut.front = NULL;
    fileManager->cut = fileManager->selectedItem;
    isCopy = false;
    if (fileManager->cut.front == NULL)
    {
        printf("Gagal Menyalin File!\n");
        return;
    }
    fileManager->temp = fileManager->cut;
    printf("File berhasil dipotong ke clipboard\n");
}

void pasteFile(FileManager *fileManager)
{
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
    char *path = fileManager->currentPath;
    Item *item;
    item = (Item *)dequeue(&(fileManager->copied));
    while (item != NULL)
    {
        {
            // Cari item di tree
            Item foundItem = searchFile(fileManager, path);
            if (&foundItem == NULL)
            {
                printf("File tidak ditemukan\n");
                return;
            }
            // Simpan item ke dalam queue cut
            enqueue(&(fileManager->cut), item);
            item = dequeue(&(fileManager->copied));
        }
    }
}

char *getNameFromPath(char *path)
{
    char *name = strrchr(path, '/'); // dapatkan string yang dimulai dari karakter slash (/) terakhir
    if (name != NULL)
    {
        return name + 1; // skip karakter slash (/) terakhir
    }
    return path; // kembalikan pathnya kalau gak ada slash (/) (ini berarti sudah nama file)
};

void selectFile(FileManager *fileManager, Item item)
{
    Item *itemToSelect = alloc(Item);
    *itemToSelect = item;
    enqueue(&(fileManager->selectedItem), (void *)itemToSelect);
}

void clearSelectedFile(FileManager *fileManager)
{
    while (!is_queue_empty(fileManager->selectedItem))
    {
        Item *item;
        item = dequeue(&(fileManager->selectedItem));
        free(item);
    }
}

void deselectFile(FileManager *fileManager, Item item)
{
    Item *itemToDeselect = alloc(Item);
    *itemToDeselect = item;
    itemToDeselect = (Item *)dequeue(&(fileManager->selectedItem));
    free(itemToDeselect);
}

bool isDirectory(char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

char *_createDuplicatedFolderName(char *filePath, char *suffix)
{
    char *newPath = TextFormat("%s%s", filePath, suffix);
    if (DirectoryExists(newPath))
    {
        newPath = _createDuplicatedFolderName(newPath, suffix);
    }
    return newPath;
}

char *_createDuplicatedFileName(char *filePath, char *suffix)
{
    size_t len;
    char *extention = strrchr(filePath, '.');
    if (extention)
    {
        len = extention - filePath;
    }
    else
    {
        len = strlen(filePath);
    }
    char *nameOnly = (char *)malloc(len + 1);
    strncpy(nameOnly, filePath, len);
    nameOnly[len] = '\0';

    char *newPath = TextFormat("%s%s%s", nameOnly, suffix, extention);
    if (FileExists(newPath))
    {
        newPath = _createDuplicatedFileName(newPath, suffix);
    }
    return newPath;
}