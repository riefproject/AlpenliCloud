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
}Item;



typedef struct TrashItem {
  Item item;
  char* originalPath;     // path asli sebelum dihapus
  time_t deletedTime;     // waktu dihapus
  char* trashPath;        // path di dalam folder trash fisik
} TrashItem;

// OPERASI
Item createItem(char* name, char* path, long size, ItemType type, time_t created_at, time_t updated_at, time_t deleted_at);

TrashItem searchTrashItem(LinkedList trash, char* originalPath);
// Getter

// Setter


#endif // !ITEM_H


