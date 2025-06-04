#include "item.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "linked.h"

Item createItem(char* name, char* path, long size, ItemType type, time_t created_at, time_t updated_at, time_t deleted_at) {
  return (Item) {
    .name = strdup(name),
      .path = strdup(path),
      .size = size,
      .type = type,
      .selected = false,
      .created_at = created_at,
      .updated_at = updated_at,
      .deleted_at = deleted_at
  };
}

TrashItem searchTrashItem(LinkedList trash, char* originalPath) {
  TrashItem* trashItemToFind;
  Node* current;
  if (trash.head == NULL) {
    printf("[LOG] Sampah kosong\n");
    return (TrashItem) {
      .deletedTime = 0,
        .item = { 0 },
        .originalPath = NULL,
        .trashPath = NULL
    };
  }
  current = trash.head;
  if (current != NULL) {
    trashItemToFind = (TrashItem*)current->data;
    if (strcmp(originalPath, trashItemToFind->originalPath) == 0) {
      return  *trashItemToFind;
    }
    current = current->next;
  }

  printf("[LOG] Item sampah tidak ditemukan\n");

  return (TrashItem) {
    .deletedTime = 0,
      .item = { 0 },
      .originalPath = NULL,
      .trashPath = NULL
  };
}

PasteItem createPasteItem(Item item, char* originalPath) {
  return (PasteItem) {
    .item = item,
    .originalPath = strdup(originalPath),
  };
}
// TrashItem createTrashItem(Item item, char* origin) {
//   return (TrashItem) {
//     .item = item,
//       .origin = strdup(origin)
//   };
// }

