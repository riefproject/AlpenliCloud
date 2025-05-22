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

/* IS:
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
// void initFileManager(FileManager* fileManager) {
//   printf("Hello World!");

// }

int loadFile(FileManager *fileManager, char *path)
{
    DIR *dp;
    struct dirent *ep;
    time_t createTime;

    dp = opendir(path);

    if (dp != NULL)
    {
        createTime = time(NULL);
        while ((ep = readdir(dp)) != NULL)
        {
            Item data = createItem(ep->d_name, path, 0, ITEM_FILE, createTime, createTime, 0);
            insert_node(fileManager->root, data);
        }
        closedir(dp);
        return 0;
    }
    else
    {
        perror("Couldn't open the directory");
        return -1;
    }
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