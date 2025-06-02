#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "item.h"
#include "nbtree.h"
#include "queue.h"
#include "stack.h"

typedef struct Context Context;

#define alloc(T) (T *)malloc(sizeof(T));

typedef struct FileManager {
  Tree root;              // root directory
  LinkedList trash;         // root trash (deleted files)
  Stack undo;             // stack for undo operations
  Stack redo;             // stack for redo operations

  Tree treeCursor;        // current tree cursor 
  // (tree dengan root adalah direktori saat ini)

  char* currentPath;       // queue for current path
  Queue copied;            // queue for copied items
  Queue cut;               // queue for cut items
  Queue temp;              // temporary queue for operations
  LinkedList selectedItem; // linkedlist for selected item
  bool needsRefresh;

  Context *ctx;
} FileManager;

/* ========== Core File Manager Functions ========== */

/**
 * @brief Membuat instance baru dari File Manager
 * @param fileManager Pointer ke struct FileManager yang akan diinisialisasi
 * @author 
 * IS: fileManager belum terinisialisasi
 * FS: fileManager terinisialisasi dengan nilai default
 */
void createFileManager(FileManager *fileManager);

/**
 * @brief Menginisialisasi File Manager dengan data sistem file
 * @param fileManager Pointer ke struct FileManager
 * @author 
 * IS: fileManager sudah dibuat tapi belum memuat data sistem file
 * FS: fileManager termuat dengan data sistem file aktual
 */
void initFileManager(FileManager *fileManager);

/**
 * @brief Memuat struktur direktori ke dalam tree
 * @param tree Tree yang akan diisi dengan struktur direktori
 * @param path Path direktori yang akan dimuat
 * IS: Tree kosong atau belum terisi dengan struktur direktori
 * FS: Tree terisi dengan struktur direktori sesuai path
 */
void loadTree(Tree tree, char *path);

/**
 * @brief Memperbarui tampilan File Manager
 * @param fileManager Filemanager
 * IS: Tampilan File Manager mungkin tidak sesuai dengan state sistem file
 * FS: Tampilan File Manager diperbarui sesuai state sistem file terkini
 */
void refreshFileManager(FileManager *fileManager);

/* ========== File Operation Functions ========== */

/**
 * @brief Mencari file dalam sistem file
 * @param fileManager Pointer ke FileManager
 * @param path Path file yang dicari
 * @return Item yang ditemukan atau NULL jika tidak ditemukan
 * @author 
 * IS: Path file yang akan dicari sudah ditentukan
 * FS: Mengembalikan Item jika ditemukan, NULL jika tidak
 */
Item searchFile(FileManager *fileManager, char *path);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void createFile(FileManager* fileManager, ItemType type, char* dirPath, char* name, bool isOperation);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void deleteFile(FileManager* fileManager, bool isOperation);

/*  Prosedur
 *  IS:
 *  FS:
================================================================================*/
void renameFile(FileManager* fileManager, char* filePath, char* newName, bool isOperation);

/**
 * @brief Memulihkan file dari trash
 * @param fileManager Pointer ke FileManager
 * IS: File berada di trash
 * FS: File dipulihkan ke lokasi asli atau lokasi yang ditentukan
 */
void recoverFile(FileManager *fileManager);

/**
 * @brief Menyalin file atau folder yang dipilih
 * @param fileManager Pointer ke FileManager
 * IS: File/folder sudah dipilih untuk disalin
 * FS: File/folder tersimpan dalam antrian copied
 */
void copyFile(FileManager *fileManager);

/**
 * @brief Menempelkan file atau folder yang telah disalin
 * @param fileManager Pointer ke FileManager
 * IS: Terdapat file/folder dalam antrian copied atau cut
 * FS: File/folder ditempel ke lokasi yang aktif
 */
void pasteFile(FileManager *fileManager);

/**
 * @brief Memotong file atau folder yang dipilih
 * @param fileManager Pointer ke FileManager
 * IS: File/folder sudah dipilih untuk dipindahkan
 * FS: File/folder tersimpan dalam antrian cut
 */
void cutFile(FileManager *fileManager);

/* ========== History Management Functions ========== */

/**
 * @brief Membatalkan operasi terakhir
 * @param fileManager Pointer ke FileManager
 * IS: Terdapat operasi yang dapat dibatalkan dalam stack undo
 * FS: Operasi terakhir dibatalkan dan dipindah ke stack redo
 */
void undo(FileManager *fileManager);

/**
 * @brief Mengulangi operasi yang dibatalkan
 * @param fileManager Pointer ke FileManager
 * IS: Terdapat operasi dalam stack redo
 * FS: Operasi teratas dari stack redo dijalankan kembali
 */
void redo(FileManager *fileManager);

/* ========== Selection Management Functions ========== */

/**
 * @brief Memilih sebuah file atau folder
 * @param fileManager Pointer ke FileManager
 * @param item Item yang akan dipilih
 * IS: Item belum terpilih
 * FS: Item ditambahkan ke daftar item yang dipilih
 */
void selectFile(FileManager *fileManager, Item *item);

/**
 * @brief Membatalkan pilihan sebuah file atau folder
 * @param fileManager Pointer ke FileManager
 * @param item Item yang akan dibatalkan pilihannya
 * IS: Item sudah terpilih
 * FS: Item dihapus dari daftar item yang dipilih
 */
void deselectFile(FileManager *fileManager, Item *item);

/**
 * @brief Membersihkan semua pilihan file
 * @param fileManager Pointer ke FileManager
 * IS: Mungkin ada item yang terpilih
 * FS: Tidak ada item yang terpilih
 */
void clearSelectedFile(FileManager *fileManager);

/**
 * @brief Memilih semua item dalam direktori aktif
 * @param fileManager Pointer ke FileManager
 * IS: Beberapa atau tidak ada item yang terpilih
 * FS: Semua item dalam direktori aktif terpilih
 */
void selectAll(FileManager *fileManager);

/* ========== Navigation Functions ========== */

/**
 * @brief Mendapatkan root dari direktori aktif
 * @param fileManager Record FileManager
 * @return Tree Root direktori yang sedang aktif
 */
Tree getCurrentRoot(FileManager fileManager);

/**
 * @brief Mendapatkan path absolut dari posisi tree
 * @param tree Tree yang akan dicari pathnya
 * @return char* Path absolut dari tree
 */
char *getCurrentPath(Tree tree);

/**
 * @brief Kembali ke direktori sebelumnya
 * @param fileManager Pointer ke FileManager
 * IS: Berada di suatu direktori
 * FS: Kembali ke direktori parent
 */
void goBack(FileManager *fileManager);

/**
 * @brief Pindah ke direktori yang dituju
 * @param fileManager Pointer ke FileManager
 * @param tree Tree tujuan
 * IS: Berada di suatu direktori
 * FS: Berpindah ke direktori tujuan
 */
void goTo(FileManager *fileManager, Tree tree);

/**
 * @brief Mengurutkan children dalam tree
 * @param parent Pointer ke tree yang akan diurutkan childrennya
 * IS: Children mungkin tidak terurut
 * FS: Children terurut sesuai kriteria (nama/tipe)
 */
void sort_children(Tree *parent);

/* ========== Helper Functions (PRIVATE) ========== */

/**
 * @brief Memindahkan item ke trash
 * @param fileManager Pointer ke FileManager
 * @param itemTree Tree item yang akan dipindahkan
 * @author 
 * IS: Item berada di sistem file
 * FS: Item dipindahkan ke trash
 */
void _moveToTrash(FileManager *fileManager, Tree itemTree);

/**
 * @brief Menghapus item secara permanen
 * @param fullPath Path lengkap item
 * @param type Tipe item
 * @param name Nama item
 * IS: Item ada di sistem file atau trash
 * FS: Item terhapus permanen dari sistem
 */
void _deletePermanently(char *fullPath, ItemType type, char *name);

/**
 * @brief Menghapus satu item
 * @param fullPath Path lengkap item
 * @param type Tipe item
 * @param name Nama item
 * IS: Item ada di sistem file
 * FS: Item terhapus dari sistem file
 */
void _deleteSingleItem(char *fullPath, ItemType type, char *name);

/**
 * @brief Menyalin isi file
 * @param srcPath Path sumber
 * @param destPath Path tujuan
 * IS: File sumber ada
 * FS: Isi file tersalin ke tujuan
 */
void _copyFileContent(char *srcPath, char *destPath);

/**
 * @brief Menyalin folder secara rekursif
 * @param srcPath Path sumber
 * @param destPath Path tujuan
 * IS: Folder sumber ada
 * FS: Folder dan isinya tersalin ke tujuan
 */
void _copyFolderRecursive(char *srcPath, char *destPath);

/**
 * @brief Menghapus item dari trash
 * @param fileManager Pointer ke FileManager
 * @param itemName Nama item yang akan dihapus
 * IS: Item ada di trash
 * FS: Item terhapus dari trash
 */
void _removeFromTrash(FileManager *fileManager, char *itemName);

/**
 * @brief Mengembalikan item ke tree
 * @param fileManager Pointer ke FileManager
 * @param trashItem Item dari trash
 * @param recoverPath Path tujuan pemulihan
 * IS: Item ada di trash
 * FS: Item dikembalikan ke sistem file
 */
void _addBackToTree(FileManager *fileManager, TrashItem *trashItem, char *recoverPath);

/**
 * @brief Menghapus node dari tree
 * @param root Root dari tree
 * @param nodeToRemove Node yang akan dihapus
 * IS: Node ada di tree
 * FS: Node terhapus dari tree
 */
void remove_node(Tree *root, Tree nodeToRemove);

/**
 * @brief Membangun ulang struktur tree
 * @param fileManager Pointer ke FileManager
 * @param sourceTree Tree sumber
 * @param newBasePath Path dasar baru
 * @param destinationPath Path tujuan
 * IS: Tree sumber ada
 * FS: Struktur tree terbangun ulang di lokasi baru
 */
void _reconstructTreeStructure(FileManager *fileManager, Tree sourceTree, char *newBasePath, char *destinationPath);

/**
 * @brief Memuat tree dari path
 * @param parentNode Node parent
 * @param basePath Path dasar
 * IS: Path valid
 * FS: Tree termuat dari path
 */
void _loadTreeFromPath(Tree parentNode, char *basePath);

/**
 * @brief Mendapatkan nama file dari path
 * @param path Path lengkap
 * @return char* Nama file
 */
char *_getNameFromPath(char *path);

/**
 * @brief Membuat nama folder duplikat
 * @param dirPath Path direktori
 * @param suffix Akhiran untuk nama
 * @return char* Nama folder baru
 */
char *_createDuplicatedFolderName(char *dirPath, char *suffix);

/**
 * @brief Membuat nama file duplikat
 * @param filePath Path file
 * @param suffix Akhiran untuk nama
 * @return char* Nama file baru
 */
char *_createDuplicatedFileName(char *filePath, char *suffix);

/**
 * @brief Mengecek apakah path adalah direktori
 * @param path Path yang akan dicek
 * @return bool true jika direktori
 */
bool isDirectory(char *path);

/**
 * @brief Mendapatkan direktori dari path lengkap
 * @param path Path lengkap
 * @return char* Path direktori
 */
char *_getDirectoryFromPath(char *path);

/**
 * @brief Membuka file dengan aplikasi default Windows
 * @param path Path file yang akan dibuka
 * IS: File ada
 * FS: File terbuka dengan aplikasi default
 */
void windowsOpenWith(char *path);

#endif // !FILE_MANAGER_H