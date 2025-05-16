#include <stdio.h>
#include <stdlib.h>
#include "nbtree.h"

void create_tree(Tree* tree) {
    *tree = NULL;
}

Node* create_node(treeInfotype data) {
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode == NULL) return NULL;
  
    newNode->item = data;
    newNode->next_brother = NULL;
    newNode->first_son = NULL;
    newNode->parent = NULL;
    return newNode;
}
  
void insert_node(Tree parent, Node *child) {
    if (parent == NULL || child == NULL) return;
  
    child->parent = parent;
  
    if (parent->first_son == NULL) {
      parent->first_son = child;
    } else {
      Tree sibling = parent->first_son;
      while (sibling->next_brother != NULL)
        sibling = sibling->next_brother;
      sibling->next_brother = child;
    }
}
  