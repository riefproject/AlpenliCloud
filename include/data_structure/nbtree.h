#ifndef NBTREE_H
#define NBTREE_H
#include "item.h"

typedef Item treeInfotype;
typedef struct TreeNode* Tree;
typedef struct TreeNode{
  treeInfotype item;
  Tree next_brother, first_son, parent;
}TreeNode;

void create_tree(Tree* tree);

Tree create_node_tree(treeInfotype data);

Tree insert_node(Tree parent, treeInfotype data);

Tree searchTree(Tree root, treeInfotype data);

void printIndent(int depth);

void printTree(Tree node, int depth);


#endif // !NBTREE_H