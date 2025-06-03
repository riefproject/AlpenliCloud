#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "item.h"
#include "nbtree.h"
#include "queue.h"
#include "stack.h"

typedef struct Context Context;

#define alloc(T) (T *)malloc(sizeof(T))

/*
 * Struktur utama untuk mengelola sistem file dalam aplikasi
 * Berisi tree direktori, trash, undo/redo, dan path aktif
 * Author: 
================================================================================*/
typedef struct FileManager {
    Tree root;               // Root direktori
    LinkedList trash;        // Trash (file yang dihapus)
    Stack undo;              // Stack undo
    Stack redo;              // Stack redo
    Tree treeCursor;         // Pointer ke direktori aktif
    char *currentPath;       // Path aktif
    Queue copied;            // Queue item disalin
    Queue cut;               // Queue item dipotong
    Queue temp;              // Queue sementara
    LinkedList selectedItem; // Daftar item yang dipilih
    bool needsRefresh;       // Flag pembaruan tampilan
    Context *ctx;            // Konteks UI
} FileManager;

/* ========== Core File Manager Functions ========== */

/*
 * IS  : fileManager belum terinisialisasi
 * FS  : fileManager terinisialisasi dengan nilai default
 * Author: 
================================================================================*/
void createFileManager(FileManager *fileManager);

/*
 * IS  : fileManager sudah dibuat tapi belum memuat data sistem file
 * FS  : fileManager termuat dengan data sistem file aktual
 * Author: 
================================================================================*/
void initFileManager(FileManager *fileManager);

/*
 * IS  : Tree kosong atau belum berisi struktur direktori
 * FS  : Tree terisi dengan struktur direktori dari path
 * Author: 
================================================================================*/
void loadTree(Tree tree, char *path);
void printTrash(LinkedList trash);
/*
 * IS  : Trash belum dimuat
 * FS  : Trash terisi dengan data dari file penyimpanan trash
 * Author: 
================================================================================*/
void loadTrashFromFile(LinkedList *trash);

/*
 * IS  : Trash berisi item yang telah dihapus
 * FS  : Data trash disimpan ke file untuk pemulihan
 * Author: 
================================================================================*/
void saveTrashToFile(FileManager *fileManager);

/*
 * IS  : Tampilan FileManager tidak sesuai state
 * FS  : Tampilan diperbarui agar sesuai dengan sistem file terkini
 * Author: 
================================================================================*/
void refreshFileManager(FileManager *fileManager);

/* ========== File Operation Functions ========== */

/*
 * IS  : Path file telah ditentukan
 * FS  : Mengembalikan item jika ditemukan, NULL jika tidak
 * Author: 
================================================================================*/
Item searchFile(FileManager *fileManager, char *path);

/*
 * IS  : Nama dan path file/folder baru telah ditentukan
 * FS  : File/folder dibuat di lokasi tersebut
 * Author: 
================================================================================*/
void createFile(FileManager *fileManager, ItemType type, char *dirPath, char *name, bool isOperation);

/*
 * IS  : File/folder telah dipilih
 * FS  : File/folder dihapus dan dipindahkan ke trash
 * Author: 
================================================================================*/
void deleteFile(FileManager *fileManager, bool isOperation);

/*
 * IS  : Nama file belum berubah
 * FS  : Nama file berubah sesuai dengan input
 * Author: 
================================================================================*/
void renameFile(FileManager *fileManager, char *filePath, char *newName, bool isOperation);

/*
 * IS  : File berada di trash
 * FS  : File dipulihkan ke lokasi aslinya
 * Author: 
================================================================================*/
void recoverFile(FileManager *fileManager);

/*
 * IS  : File/folder telah dipilih untuk disalin
 * FS  : Tersimpan dalam antrian copied
 * Author: 
================================================================================*/
void copyFile(FileManager *fileManager);

/*
 * IS  : File/folder telah disalin/dipotong
 * FS  : File/folder ditempel ke direktori aktif
 * Author: 
================================================================================*/
void pasteFile(FileManager *fileManager);

/*
 * IS  : File/folder telah dipilih untuk dipindahkan
 * FS  : Tersimpan dalam antrian cut
 * Author: 
================================================================================*/
void cutFile(FileManager *fileManager);

/* ========== History Management Functions ========== */

/*
 * IS  : Terdapat operasi dalam undo stack
 * FS  : Operasi dibatalkan dan masuk ke redo stack
 * Author: 
================================================================================*/
void undo(FileManager *fileManager);

/*
 * IS  : Terdapat operasi dalam redo stack
 * FS  : Operasi terakhir redo dijalankan kembali
 * Author: 
================================================================================*/
void redo(FileManager *fileManager);

/* ========== Selection Management Functions ========== */

/*
 * IS  : Item belum dipilih
 * FS  : Item masuk ke daftar selected
 * Author: 
================================================================================*/
void selectFile(FileManager *fileManager, Item *item);

/*
 * IS  : Item sudah dipilih
 * FS  : Item dihapus dari daftar selected
 * Author: 
================================================================================*/
void deselectFile(FileManager *fileManager, Item *item);

/*
 * IS  : Mungkin ada item yang dipilih
 * FS  : Semua pilihan dibersihkan
 * Author: 
================================================================================*/
void clearSelectedFile(FileManager *fileManager);

/*
 * IS  : Beberapa atau tidak ada item terpilih
 * FS  : Semua item pada direktori aktif terpilih
 * Author: 
================================================================================*/
void selectAll(FileManager *fileManager);

/* ========== Navigation Functions ========== */

/*
 * IS  : FileManager aktif
 * FS  : Mengembalikan root direktori aktif
 * Author: 
================================================================================*/
Tree getCurrentRoot(FileManager fileManager);

/*
 * IS  : Tree sudah valid
 * FS  : Mengembalikan path absolut dari tree
 * Author: 
================================================================================*/
char *getCurrentPath(Tree tree);

/*
 * IS  : Berada di direktori apapun
 * FS  : Pindah ke direktori parent
 * Author: 
================================================================================*/
void goBack(FileManager *fileManager);

/*
 * IS  : Berada di direktori manapun
 * FS  : Pindah ke tree direktori tujuan
 * Author: 
================================================================================*/
void goTo(FileManager *fileManager, Tree tree);

/*
 * IS  : Children belum terurut
 * FS  : Children terurut berdasarkan nama/tipe
 * Author: 
================================================================================*/
void sort_children(Tree *parent);

/* ========== Helper Functions (PRIVATE) ========== */

/*
 * IS  : Item berada di tree sistem file
 * FS  : Item dipindahkan ke trash
 * Author: 
================================================================================*/
void _moveToTrash(FileManager *fileManager, Tree itemTree);

/*
 * IS  : Item ada di sistem file
 * FS  : Item dihapus permanen
 * Author: 
================================================================================*/
void _deletePermanently(char *fullPath, ItemType type, char *name);

/*
 * IS  : Item ada di sistem file
 * FS  : Item dihapus dari sistem file
 * Author: 
================================================================================*/
void _deleteSingleItem(char *fullPath, ItemType type, char *name);

/*
 * IS  : File sumber ada
 * FS  : Isi file tersalin ke path tujuan
 * Author: 
================================================================================*/
void _copyFileContent(char *srcPath, char *destPath);

/*
 * IS  : Folder sumber ada
 * FS  : Folder dan isinya tersalin ke tujuan
 * Author: 
================================================================================*/
void _copyFolderRecursive(char *srcPath, char *destPath);

/*
 * IS  : Item ada di trash
 * FS  : Item dihapus dari trash
 * Author: 
================================================================================*/
void _removeFromTrash(FileManager *fileManager, char *itemName);

/*
 * IS  : Item ada di trash
 * FS  : Item dikembalikan ke tree
 * Author: 
================================================================================*/
void _addBackToTree(FileManager *fileManager, TrashItem *trashItem, char *recoverPath);

/*
 * IS  : Node valid dalam tree
 * FS  : Node dihapus dari tree
 * Author: 
================================================================================*/
void remove_node(Tree *root, Tree nodeToRemove);

/*
 * IS  : Source tree valid
 * FS  : Struktur tree disalin ke lokasi baru
 * Author: 
================================================================================*/
void _reconstructTreeStructure(FileManager *fileManager, Tree sourceTree, char *newBasePath, char *destinationPath);

/*
 * IS  : Path valid
 * FS  : Tree termuat dari path
 * Author: 
================================================================================*/
void _loadTreeFromPath(Tree parentNode, char *basePath);

/*
 * IS  : Path valid
 * FS  : Mengembalikan nama file dari path
 * Author: 
================================================================================*/
char *_getNameFromPath(char *path);

/*
 * IS  : Path direktori valid
 * FS  : Menghasilkan nama folder duplikat
 * Author: 
================================================================================*/
char *_createDuplicatedFolderName(char *dirPath, char *suffix);

/*
 * IS  : Path file valid
 * FS  : Menghasilkan nama file duplikat
 * Author: 
================================================================================*/
char *_createDuplicatedFileName(char *filePath, char *suffix);

/*
 * IS  : Path valid
 * FS  : Mengembalikan true jika path adalah direktori
 * Author: 
================================================================================*/
bool isDirectory(char *path);

/*
 * IS  : Path valid
 * FS  : Mengembalikan direktori dari path
 * Author: 
================================================================================*/
char *_getDirectoryFromPath(char *path);

/*
 * IS  : File ada dan bisa dibuka
 * FS  : File dibuka dengan aplikasi default Windows
 * Author: 
================================================================================*/
void windowsOpenWith(char *path);

#endif // !FILE_MANAGER_H
