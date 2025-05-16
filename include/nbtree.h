#ifndef NBTREE_H
#define NBTREE_H
#include "item.h"

typedef Item treeInfotype;
typedef struct Node* Tree;
typedef struct Node{
  treeInfotype item;
  Tree next_brother, first_son, parent;
}Node;

void create_tree(Tree* tree);

Node* create_node(treeInfotype data);

void insert_node(Tree parent, treeInfotype data);


#endif // !NBTREE_H