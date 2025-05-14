#ifndef ITEM_H
#define ITEM_H
#include <time.h>

typedef enum ItemType{
  FOLDER,
  FILE
} ItemType;

typedef struct Item{
  char* name;
  size_t size;
  ItemType type;
  time_t created_at, updated_at, deleted_at;
}Item;

// OPERASI

// Getter

// Setter


#endif // !ITEM_H