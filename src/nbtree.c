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
  
void insert_node(Tree parent, treeInfotype data) {
    Node* child = create_node(data);
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

Tree searchTree(Tree root, treeInfotype item){
  // Kalau tree kosoong, return null
  if(root == NULL) return NULL;
  // kalau root sama dengan item yang dicari, return root
  if(strcmp(root->item.name, item.name) == 0 && strcmp(root->item.path, item.path) == 0) return root;
  // cari di first_son
  Tree found = searchTree(root->first_son, item);
  // kalau ketemu, return found
  if(found != NULL) return found;
  // cari di next_brother
  return searchTree(root->next_brother, item);
}