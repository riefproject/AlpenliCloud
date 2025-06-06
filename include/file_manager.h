#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "item.h"
#include "nbtree.h"
#include "queue.h"
#include "stack.h"
#include <stdbool.h>
#include "operation.h"

#define alloc(T) (T *)malloc(sizeof(T))

typedef struct Context Context;
/*
 * Struktur utama untuk mengelola sistem file dalam aplikasi
 * Berisi tree direktori, trash, undo/redo, dan path aktif
 * Author:
================================================================================*/
typedef struct FileManager {
    // (tree dengan root adalah direktori saat ini)
    Tree root;        // root directory
    LinkedList trash; // root trash (deleted files)
    bool isRootTrash; // status apakah trash adalah root trash

    LinkedList searchingList; // linkedlist untuk menyimpan hasil pencarian
    bool isSearching;         // status apakah sedang dalam mode pencarian

    char* currentPath; // current path string
    Tree treeCursor;   // current tree cursor

    Stack undo;              // stack for undo operations
    Stack redo;              // stack for redo operations
    Queue copied;            // queue for copied items
    Queue clipboard;         // temporary queue for operations
    LinkedList selectedItem; // linkedlist for selected item

    bool needsRefresh;
    Context* ctx; // context untuk akses sidebar dan komponen lainnya
} FileManager;

/*
====================================================================
    INISIALISASI DAN SETUP
====================================================================
*/

// Prosedur membuat file manager baru
// Menginisialisasi semua struktur data dalam FileManager dengan nilai kosong
// IS: FileManager belum terinisialisasi
// FS: Semua struktur data (tree, stack, queue, list) telah dibuat dan bernilai kosong/NULL
// Created by: Maulana
void createFileManager(FileManager* fileManager);

// Prosedur inisialisasi file manager
// Memuat root directory dari filesystem dan mengatur treeCursor awal
// IS: FileManager telah dibuat tetapi belum dimuat dengan data filesystem
// FS: Root directory dimuat dari ROOT path, treeCursor menunjuk ke root, currentPath diset ke "root"
// Created by: Farras
void initFileManager(FileManager* fileManager);

// Function memuat struktur tree
// Membaca direktori dan file dari filesystem secara rekursif untuk membangun struktur tree
// IS: Tree dan path direktori diketahui
// FS: Tree berisi struktur file dan folder sesuai dengan isi direktori di path secara rekursif
// Created by: Farras
void loadTree(Tree tree, char* path);

// Prosedur load trash dari file
// Membaca file trash_dump.txt untuk memuat item yang telah dihapus ke dalam LinkedList trash
// IS: File trash_dump.txt ada dan berisi data yang valid
// FS: LinkedList trash diisi dengan item yang telah dihapus, setiap item berisi nama, path asli, waktu penghapusan, dan path trash
// Created by: Farras
void loadTrashFromFile(LinkedList* trash);

// Prosedur untuk menimpan trash ke file
// Menyimpan semua item yang ada di LinkedList trash ke file trash_dump.txt
// IS: LinkedList trash berisi item yang telah dihapus
// FS: File trash_dump.txt diupdate dengan data dari LinkedList trash, setiap item disimpan dalam format "name,originalPath,deletedTime"
// Created by: Farras
void saveTrashToFile(FileManager* fileManager);

// Prosedur untuk mencetak isi trash
// Mencetak semua item yang ada di LinkedList trash ke konsol
// IS: LinkedList trash berisi item yang telah dihapus
// FS: Mencetak setiap item dengan nama, path asli, waktu penghapusan, dan path trash ke konsol
// Created by: Farras
void printTrash(LinkedList trash);

// Prosedur refresh file manager
// Memuat ulang struktur tree dari direktori saat ini dengan menghapus child dan reload
// IS: Struktur tree mungkin tidak sinkron dengan filesystem
// FS: Child nodes dihapus, tree dimuat ulang dari path treeCursor, struktur tree sinkron dengan filesystem
// Created by: Arief
// Edited by: Farras
void refreshFileManager(FileManager* fileManager);

/*
====================================================================
    OPERASI FILE DAN FOLDER
====================================================================
*/

// Function search file
// Mencari file berdasarkan path dengan membuat item dummy dan menggunakan searchTree
// IS: Path file diketahui dan valid
// FS: Item file ditemukan dan dikembalikan, atau item kosong dengan semua field 0/NULL jika tidak ditemukan
// Created by: Maulana
Item searchFile(FileManager* fileManager, char* path);

// Prosedur search all file/folder
// Mencari semua file/folder yang sesuai dengan keyword di direktori saat ini
// IS: Keyword untuk pencarian diketahui
// FS: LinkedList searchingList diisi dengan item yang sesuai, setiap item berisi nama, path, ukuran, tipe, dan waktu
// Created by: Farras
void searchingTreeItem(FileManager* fileManager, char* keyword);

// Prosedur search item di tree secara rekursif
// Mencari semua item yang sesuai dengan keyword di tree secara rekursif
// IS: LinkedList untuk menyimpan hasil pencarian, tree yang akan dicari, dan keyword yang digunakan
// FS: LinkedList diisi dengan item yang sesuai, setiap item berisi nama, path, ukuran, tipe, dan waktu
// Created by: Farras
void searchingTreeItemRecursive(LinkedList* linkedList, Tree tree, const char* keyword);

// Prosedur search item di linked list
// Mencari item di LinkedList berdasarkan keyword dengan membandingkan nama item
// IS: LinkedList yang berisi item, item yang akan dicari, dan keyword yang digunakan
// FS: LinkedList diisi dengan item yang sesuai, setiap item berisi nama, path, ukuran, tipe, dan waktu
// Created by: Farras
void searchingLinkedListItem(FileManager* FileManager, Node* node, char* keyword);

// Prosedur search item di linked list secara rekursif
// Mencari item di LinkedList secara rekursif berdasarkan keyword
// IS: LinkedList yang berisi item, node yang akan dicari, dan keyword yang digunakan
// FS: LinkedList diisi dengan item yang sesuai, setiap item berisi nama, path, ukuran, tipe, dan waktu
// Created by: Farras
void searchingLinkedListRecursive(FileManager* FileManager, Node* node, char* keyword);

// Prosedur print searching list
// Mencetak semua item yang ada di LinkedList searchingList ke konsol
// IS: LinkedList searchingList berisi item yang telah ditemukan
// FS: Mencetak setiap item dengan nama, path, ukuran, tipe, dan waktu ke konsol
// Created by: Farras
void printSearchingList(FileManager* fileManager);

// Prosedur create file/folder
// Membuat file atau folder baru di direktori dengan pengecekan duplikasi dan operasi undo
// IS: Tipe, path direktori, dan nama item baru diketahui
// FS: File/folder dibuat di filesystem, ditambahkan ke tree, operasi disimpan untuk undo jika isOperation=true
// Created by: Maulana
void createFile(FileManager* fileManager, ItemType type, char* dirPath, char* name, bool isOperation);

// Prosedur delete file/folder ke trash
// Memindahkan semua file/folder yang dipilih ke direktori trash fisik dan LinkedList trash
// IS: Ada file/folder di selectedItem yang dipilih untuk dihapus
// FS: File/folder dipindahkan ke direktori trash, ditambahkan ke LinkedList trash, dihapus dari tree utama, operasi disimpan untuk undo
// Created by: Arief
void deleteFile(FileManager* fileManager, bool isOperation);

// Prosedur rename file/folder
// Mengganti nama file atau folder dengan pengecekan duplikasi dan operasi filesystem
// IS: Path file lama dan nama baru diketahui
// FS: File/folder direname di filesystem, item di tree diupdate dengan nama dan path baru, operasi disimpan untuk undo jika isOperation=true
// Created by: Maulana
void renameFile(FileManager* fileManager, char* filePath, char* newName, bool isOperation);

// Prosedur recover file dari trash
// Mengembalikan file/folder yang dipilih dari LinkedList trash ke tree utama dan filesystem
// IS: Ada file/folder di selectedItem yang ada di LinkedList trash
// FS: File/folder dipindahkan dari trash ke lokasi recovery, dihapus dari LinkedList trash, ditambahkan kembali ke tree
// Created by: Arief
void recoverFile(FileManager* fileManager);

/*
====================================================================
    OPERASI COPY, CUT, PASTE
====================================================================
*/

// Prosedur copy file/folder
// Menyalin semua file/folder yang dipilih ke queue copied dan mengatur mode copy
// IS: Ada file/folder yang dipilih di selectedItem
// FS: File/folder disalin ke queue copied, isCopy diset true, queue temp berisi data copied
// Created by: Arief
void copyFile(FileManager* fileManager);

// Prosedur cut file/folder
// Memotong semua file/folder yang dipilih ke queue copied dan mengatur mode cut
// IS: Ada file/folder yang dipilih di selectedItem
// FS: File/folder disalin ke queue copied, isCopy diset false, queue temp berisi data copied
// Created by: Arief
void cutFile(FileManager* fileManager);

// Prosedur paste file/folder
// Menempelkan file/folder dari queue temp ke direktori saat ini dengan progress bar untuk item banyak
// IS: Ada file/folder di queue temp dari operasi copy/cut
// FS: File/folder ditempelkan ke treeCursor dengan copy content atau move, progress bar ditampilkan jika >10 item, queue temp dikosongkan untuk cut
// Created by: Arief
void pasteFile(FileManager *fileManager, bool isOperation);


/*
====================================================================
    OPERASI SELEKSI FILE
====================================================================
*/

// Prosedur select file/folder
// Menambahkan item ke LinkedList selectedItem jika belum ada
// IS: Item belum dipilih atau sudah dipilih
// FS: Item ditambahkan ke selectedItem jika belum ada, item.selected diset true
// Created by:
void selectFile(FileManager* fileManager, Item* item);

// Prosedur deselect file/folder
// Menghapus item dari LinkedList selectedItem dan mengatur selected=false
// IS: Item mungkin ada di selectedItem
// FS: Item dihapus dari selectedItem jika ditemukan, item.selected diset false, LinkedList di-reset jika kosong
// Created by:
void deselectFile(FileManager* fileManager, Item* item);

// Prosedur clear selected files
// Menghapus semua item dari LinkedList selectedItem dengan memanggil deselectFile untuk setiap item
// IS: Ada item yang dipilih di selectedItem
// FS: Semua item di-deselect, selectedItem menjadi kosong
// Created by:
void clearSelectedFile(FileManager* fileManager);

// Prosedur select all files
// Memilih semua child nodes dari treeCursor saat ini
// IS: treeCursor menunjuk ke direktori yang valid dengan child nodes
// FS: Semua child nodes dari treeCursor dipilih dan ditambahkan ke selectedItem
// Created by:
void selectAll(FileManager* fileManager);

/*
====================================================================
    OPERASI UNDO DAN REDO
====================================================================
*/

// Prosedur undo operasi
// Membatalkan operasi terakhir dari stack undo sesuai dengan tipe operasi
// IS: Ada operasi di stack undo
// FS: Operasi dibatalkan sesuai tipe (CREATE→delete, DELETE→restore, UPDATE→rename back), operasi dipindahkan ke stack redo, filesystem di-refresh
// Created by: Maulana
void undo(FileManager* fileManager);

// Prosedur redo operasi
// Mengulangi operasi yang sebelumnya dibatalkan dari stack redo
// IS: Ada operasi di stack redo
// FS: Operasi dijalankan kembali sesuai tipe, operasi dipindahkan kembali ke stack undo
// Created by: Maulana
void redo(FileManager* fileManager);

/*
====================================================================
    NAVIGASI DIREKTORI
====================================================================
*/

// Function get current root
// Mencari dan mengembalikan root directory dari treeCursor dengan traversal ke parent
// IS: FileManager dan treeCursor valid
// FS: Root directory ditemukan dan dikembalikan, atau NULL jika invalid
// Created by: Farras
Tree getCurrentRoot(FileManager fileManager);

// Function get current path
// Membangun string path lengkap dari tree node dengan traversal ke parent
// IS: Tree node valid
// FS: String path lengkap dikembalikan dengan format "root/folder1/folder2"
// Created by: Farras
char* getCurrentPath(Tree tree);

// Prosedur go back directory
// Pindah treeCursor ke parent directory jika ada
// IS: treeCursor tidak berada di root directory
// FS: treeCursor pindah ke parent directory, currentPath diperbarui, filesystem di-refresh
// Created by: Farras
void goBack(FileManager* fileManager);

// Prosedur go to directory
// Pindah treeCursor ke tree target dan update currentPath
// IS: Tree target valid dan dapat diakses
// FS: treeCursor pindah ke tree target, currentPath diperbarui dengan path lengkap, filesystem di-refresh
// Created by: Farras
void goTo(FileManager* FileManager, Tree tree);

// Prosedur sort children
// Mengurutkan child nodes berdasarkan tipe menggunakan insertion sort (folder dulu, lalu file)
// IS: Parent node memiliki child nodes yang belum terurut
// FS: Child nodes diurutkan berdasarkan ItemType (ITEM_FOLDER < ITEM_FILE)
// Created by: Farras
void sort_children(Tree* parent);

/*
====================================================================
    UTILITY DAN HELPER FUNCTIONS
====================================================================
*/

// Prosedur open with windows
// Membuka dialog "Open With" Windows untuk file tertentu menggunakan command line
// IS: Path file valid dan dapat diakses
// FS: Command "cmd /c start path /OPENAS" dijalankan, dialog "Open With" Windows terbuka
// Created by: Farras
void windowsOpenWith(char* path);

// Function check progress bar requirement
// Menentukan apakah perlu menampilkan progress bar berdasarkan threshold 10 item
// IS: totalItems diketahui
// FS: Return true jika totalItems > 10, false jika ≤ 10
// Created by: Arief
bool shouldShowProgressBar(int totalItems);

/*
====================================================================
    HELPER FUNCTIONS (PRIVATE)
====================================================================
*/

// Prosedur move to trash
// Memindahkan file/folder dari tree utama ke direktori trash fisik dan LinkedList trash
// IS: ItemTree ada di tree utama dan filesystem
// FS: Item dipindahkan ke direktori ".dir/trash", TrashItem dibuat dan ditambahkan ke LinkedList trash, item dihapus dari tree utama
// Created by: Arief
void _moveToTrash(FileManager* fileManager, Tree itemTree);

// Prosedur delete permanently
// Menghapus file/folder secara permanen dari filesystem tanpa ke trash
// IS: Item ada di filesystem dengan path dan tipe yang valid
// FS: Item dihapus permanen dari filesystem (file dengan remove, folder dengan RemoveItemsRecurse)
// Created by: Arief
void _deletePermanently(char* fullPath, ItemType type, char* name);

// Prosedur delete single item
// Wrapper untuk _deletePermanently, digunakan khusus untuk operasi cut
// IS: Item ada di filesystem dengan path dan tipe yang valid
// FS: Item dihapus permanen dari filesystem menggunakan _deletePermanently
// Created by: Arief
void _deleteSingleItem(char* fullPath, ItemType type, char* name);

// Prosedur copy file content
// Menyalin isi file dari sumber ke tujuan menggunakan buffer 4096 bytes
// IS: File sumber dapat dibaca, path tujuan valid untuk ditulis
// FS: Konten file disalin byte per byte dari source ke destination menggunakan fread/fwrite
// Created by: Arief
void _copyFileContent(char* srcPath, char* destPath);

// Prosedur copy folder recursive
// Menyalin folder dan semua isinya secara rekursif dengan manual string construction
// IS: Folder sumber ada dan dapat dibaca, path tujuan valid
// FS: Folder destination dibuat, semua file dan subfolder disalin rekursif dari source ke destination
// Created by: Arief
void _copyFolderRecursive(char* srcPath, char* destPath);

// Prosedur remove from trash
// Mencari dan menghapus TrashItem dari LinkedList trash berdasarkan nama item
// IS: Item ada di LinkedList trash dengan nama yang sesuai
// FS: TrashItem dihapus dari LinkedList trash, memory untuk TrashItem dan path di-free
// Created by: Arief
void _removeFromTrash(FileManager* fileManager, char* itemName);

// Prosedur add back to tree
// Menambahkan item yang dipulihkan kembali ke tree structure di lokasi yang ditentukan
// IS: TrashItem valid, recoverPath valid, parent node dapat ditemukan
// FS: Item ditambahkan kembali ke tree sebagai child dari parent node dengan path dan nama yang diupdate
// Created by: Arief
void _addBackToTree(FileManager* fileManager, TrashItem* trashItem, char* recoverPath);

// Prosedur remove node
// Menghapus node tertentu dari struktur tree dengan mengatur ulang hubungan parent-child dan sibling
// IS: Node ada di tree structure dengan parent dan sibling yang valid
// FS: Node dihapus dari tree, hubungan parent-child dan sibling diperbarui, memory node di-free
// Created by:
void remove_node(Tree* root, Tree nodeToRemove);

// Prosedur reconstruct tree structure
// Membangun kembali struktur tree setelah operasi paste dengan membuat node baru dan load dari filesystem
// IS: Source tree dan destination path valid
// FS: Node baru dibuat di treeCursor, struktur tree direkonstruksi dengan _loadTreeFromPath
// Created by: Arief
void _reconstructTreeStructure(FileManager* fileManager, Tree sourceTree, char* newBasePath, char* destinationPath);

// Prosedur load tree from path
// Memuat struktur tree dari direktori yang sudah ada di filesystem secara rekursif
// IS: Parent node valid, basePath adalah direktori yang ada di filesystem
// FS: Tree dimuat dengan struktur sesuai filesystem, semua file dan folder ditambahkan sebagai child nodes
// Created by: Farras
void _loadTreeFromPath(Tree parentNode, char* basePath);

// Function get name from path
// Mengekstrak nama file dari path lengkap dengan mencari karakter '/' terakhir
// IS: Path lengkap dengan separator '/' atau path tanpa separator
// FS: Nama file dikembalikan tanpa path directory, atau path asli jika tidak ada separator
// Created by: Maulana
char* _getNameFromPath(char* path);

// Function create duplicated folder name
// Membuat nama folder baru dengan suffix untuk menghindari duplikasi secara rekursif
// IS: Path folder dan suffix diketahui
// FS: Nama folder unik dikembalikan dengan suffix yang sesuai, rekursif jika masih duplikat
// Created by: Maulana
char* _createDuplicatedFolderName(char* dirPath, char* suffix);

// Function create duplicated file name
// Membuat nama file baru dengan suffix sebelum ekstensi untuk menghindari duplikasi
// IS: Path file dan suffix diketahui
// FS: Nama file unik dikembalikan dengan suffix sebelum ekstensi, rekursif jika masih duplikat
// Created by: Maulana
char* _createDuplicatedFileName(char* filePath, char* suffix);

// Function check if directory
// Memeriksa apakah path yang diberikan merupakan direktori menggunakan stat dan S_ISDIR
// IS: Path diketahui dan dapat diakses
// FS: Return true jika path adalah direktori, false jika bukan direktori
// Created by: Maulana
bool isDirectory(char* path);

// Function get directory from path
// Mengekstrak path direktori dari path lengkap file dengan mencari '/' terakhir
// IS: Path lengkap file diketahui
// FS: Path direktori dikembalikan tanpa nama file, atau string kosong jika tidak ada separator
// Created by: Maulana
char* _getDirectoryFromPath(char* path);

// =================================================================================
//                      DEKLARASI FUNGSI HELPER INTERNAL
// =================================================================================

// Function undoCreate
// Membatalkan operasi pembuatan file/folder dengan menghapusnya secara permanen.
// IS: Item yang akan di-undo ada di filesystem dan tree. Operasi `op` berisi path item tersebut.
// FS: Item dihapus dari filesystem dan tree.
// Created by: Maulana
void _undoCreate(FileManager *fm, Operation *operationToUndo);

// Function undoDelete
// Membatalkan operasi penghapusan (yang memindahkan ke trash) dengan mengembalikan item ke path aslinya.
// IS: Item yang akan di-undo ada di dalam trash. `op->itemTemp` berisi daftar TrashItem.
// FS: Item dikembalikan ke path aslinya di filesystem dan tree. Daftar item dipindahkan ke `opToRedo`.
// Created by: Maulana
void _undoDelete(FileManager *fm, Operation *opToUndo, Operation *opToRedo);

// Function undoRename
// Membatalkan operasi penggantian nama dengan mengembalikannya ke nama semula.
// IS: Item ada dengan nama baru (`op->to`).
// FS: Item dikembalikan ke nama lamanya (`op->from`).
// Created by: Maulana
void _undoRename(FileManager *fm, Operation *op);

// Function undoPaste
// Membatalkan operasi paste. Jika 'cut', kembalikan ke lokasi asal. Jika 'copy', hapus hasil paste.
// IS: Item hasil paste ada di lokasi tujuan. `op` berisi detail operasi paste.
// FS: Item dikembalikan ke keadaan sebelum di-paste.
// Created by: Maulana
void _undoPaste(FileManager *fm, Operation *op, Operation *opToRedo);

// Function undoRecover
// Membatalkan operasi pemulihan dari trash dengan mengembalikan item ke trash.
// IS: Item yang dipulihkan ada di filesystem. `op->itemTemp` berisi daftar item yang di-recover.
// FS: Item dikembalikan ke trash, baik secara fisik maupun di dalam struktur data.
// Created by: Maulana
void _undoRecover(FileManager *fm, Operation *op, Operation *opToRedo);

// Function redoCreate
// Menjalankan kembali operasi pembuatan file/folder yang telah di-undo.
// IS: Item tidak ada di filesystem. `op` berisi path item yang akan dibuat.
// FS: Item dibuat kembali di filesystem dan tree.
// Created by: Maulana
void _redoCreate(FileManager *fm, Operation *op);

// Function redoDelete
// Menjalankan kembali operasi penghapusan ke trash.
// IS: Item ada di filesystem setelah di-undo. `op->itemTemp` berisi daftar item yang akan dihapus.
// FS: Item dipindahkan kembali ke trash.
// Created by: Maulana
void _redoDelete(FileManager *fm, Operation *op, Operation *opToUndo);

// Function redoRename
// Menjalankan kembali operasi penggantian nama.
// IS: Item ada dengan nama lama (`op->from`).
// FS: Item diganti namanya menjadi nama baru (`op->to`).
// Created by: Maulana
void _redoRename(FileManager *fm, Operation *op);

// Function redoPaste
// Menjalankan kembali operasi paste yang telah di-undo.
// IS: Item berada di keadaan sebelum di-paste.
// FS: Item di-paste kembali ke lokasi tujuan.
// Created by: Maulana
void _redoPaste(FileManager *fm, Operation *op, Operation *opToUndo);

// Function redoRecover
// Menjalankan kembali operasi pemulihan dari trash.
// IS: Item ada di dalam trash.
// FS: Item dipulihkan dari trash ke lokasi aslinya.
// Created by: Maulana
void _redoRecover(FileManager *fm, Operation *op, Operation *opToUndo);



int _calculateTotalPasteItems(FileManager* fileManager);
bool _processSinglePasteItem(FileManager* fileManager, Item* itemToPaste, char** originPath);
bool _pasteFolderItem(FileManager* fileManager, Item* itemToPaste, char* originPath, char* newPath);
bool _pasteFileItem(FileManager* fileManager, Item* itemToPaste, char* originPath, char* newPath);
void _addItemToCurrentTree(FileManager* fileManager, Item* itemToPaste, char* newPath, ItemType type);
void _handleCutCleanup(FileManager* fileManager, Item* itemToPaste);
void _createPasteItemRecord(Item* itemToPaste, char* originPath);

bool _isItemEqual(Item* item1, Item* item2);
void _freeSelectedNode(Node* node);

void _refreshTreeSafely(FileManager* fileManager, char* targetPath);
Tree _findNodeByPath(Tree root, char* targetPath);
void _refreshSidebarSafely(FileManager* fileManager);

bool _isValidTreeNode(Tree root, Tree target);

#endif // !FILE_MANAGER_H
