#ifndef ITEM_H
#define ITEM_H

#include <stdio.h>
#include <time.h>
#include "queue.h"

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

typedef struct TrashItem{
  Item item;
  char* origin;
} TrashItem;

// OPERASI
Item createItem(char *name, char* path, long size, ItemType type, time_t created_at, time_t updated_at, time_t deleted_at);
TrashItem createTrashItem(Item item, char* origin);
// Getter


// Setter


#endif // !ITEM_H


