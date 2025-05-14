#ifndef NTREE_H
#define NTREE_H
#include "item.h"

typedef Item infotype;
typedef struct Node* address;
typedef struct Node{
  infotype item;
  address next_brother, first_son, parent;
}Node;


#endif // !NTREE_H