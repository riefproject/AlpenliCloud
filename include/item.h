#ifndef ITEM_H
#define ITEM_H

#include <stdio.h>
#include <time.h>
#include "queue.h"
#include "linked.h"

typedef enum ItemType {
  ITEM_FOLDER,
  ITEM_FILE,
} ItemType;

typedef struct Item {
  char* name;
  char* path;
  long size;
  ItemType type;
  time_t created_at, updated_at, deleted_at;
  bool selected;
} Item;



typedef struct TrashItem {
  Item item;
  char* originalPath;     // path asli sebelum dihapus
  time_t deletedTime;     // waktu dihapus
  char* trashPath;        // path di dalam folder trash fisik
} TrashItem;

/*
====================================================================
    KONSTRUKTOR ITEM
====================================================================
*/

// Function create item
// Membuat struct Item dengan semua field terisi dan selected=false menggunakan strdup untuk string
// IS: Parameter name, path, size, type, dan waktu diketahui dan valid
// FS: Item baru dibuat dengan name dan path di-copy menggunakan strdup, selected diset false, semua field terisi sesuai parameter
// Created by: Maulana
Item createItem(char* name, char* path, long size, ItemType type, time_t created_at, time_t updated_at, time_t deleted_at);

/*
====================================================================
    OPERASI TRASH ITEM
====================================================================
*/

// Function search trash item
// Mencari TrashItem berdasarkan originalPath dalam LinkedList trash dengan strcmp
// IS: LinkedList trash dan originalPath diketahui
// FS: TrashItem yang sesuai dikembalikan jika ditemukan, atau TrashItem dengan semua field 0/NULL jika tidak ditemukan atau trash kosong
// Created by: Maulana
TrashItem searchTrashItem(LinkedList trash, char* originalPath);

#endif // !ITEM_H