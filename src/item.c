#include "item.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// typedef struct Item{
//   char* name;
//   size_t size;
//   ItemType type;
//   time_t created_at, updated_at, deleted_at;
// }Item;


Item createItem(char *name, char* path, size_t size, ItemType type, time_t created_at, time_t updated_at, time_t deleted_at){
  return (Item){
    .name = strdup(name),
    .path = strdup(path),
    .size = size,
    .type = type,
    .created_at = created_at,
    .updated_at = updated_at,
    .deleted_at = deleted_at
  };
}

TrashItem createTrashItem(Item item, char *origin){
  return (TrashItem){
    .item = item,
    .origin = strdup(origin)
  };
}
