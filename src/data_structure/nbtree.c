#include <stdio.h>
#include <stdlib.h>
#include "nbtree.h"
#include <string.h>

void create_tree(Tree* tree) {
  *tree = NULL;
}

Tree create_node_tree(treeInfotype data) {
  TreeNode* newNode = (TreeNode*)malloc(sizeof(TreeNode));
  if (newNode == NULL)
    return NULL;

  newNode->item = data;
  newNode->next_brother = NULL;
  newNode->first_son = NULL;
  newNode->parent = NULL;
  return newNode;
}

Tree insert_node(Tree parent, treeInfotype data) {
  Tree child = create_node_tree(data);

  // kembalikan jika dia root dan gagal alokasi
  if (parent == NULL || child == NULL) return NULL;

  child->parent = parent;

  if (parent->first_son == NULL) {
    parent->first_son = child;
  }
  else {
    Tree sibling = parent->first_son;
    while (sibling->next_brother != NULL)
      sibling = sibling->next_brother;
    sibling->next_brother = child;
  }

  return child;
}

Tree searchTree(Tree root, treeInfotype item) {
  if (root == NULL) return NULL;
  Tree current = root;

  while (current != NULL) {
    if (
      strcmp(current->item.name, item.name) == 0 &&
      strcmp(current->item.path, item.path) == 0) {
      return current;
    }
    current = current->next_brother;
  }

  current = root;
  while (current != NULL) {
    Tree found = searchTree(current->first_son, item);
    if (found != NULL) return found;
    current = current->next_brother;
  }

  return NULL;
}

// Fungsi bantu untuk mencetak indentasi
void printIndent(int depth) {
  for (int i = 0; i < depth; i++) {
    printf("  "); // dua spasi per level
  }
}

// Fungsi rekursif untuk mencetak pohon
void printTree(Tree node, int depth) {
  if (node == NULL)
    return;

  // Cetak node saat ini

  char* typeStr = node->item.type == ITEM_FOLDER ? "(folder)" : "(file)";

  // Format waktu
  char updated_at_str[20];
  strftime(updated_at_str, sizeof(updated_at_str), "%Y-%m-%d", localtime(&node->item.updated_at));

  printIndent(depth);
  printf("==================================\n");
  printIndent(depth);
  printf("item name: %s\n", node->item.name);
  printIndent(depth);
  printf("first son name: %s\n", node->first_son == NULL ? "NULL" : node->first_son->item.name);
  printIndent(depth);
  printf("brother name: %s\n", node->next_brother == NULL ? "NULL" : node->next_brother->item.name);
  printIndent(depth);
  printf("parent name: %s\n", node->parent == NULL ? "NULL" : node->parent->item.name);
  printIndent(depth);
  printf("size: %d\n", node->item.size);
  printIndent(depth);
  printf("path: %s\n", node->item.path);

  printIndent(depth);
  printf("==================================\n");

  // Cetak saudara (next_brother)
  if (node->next_brother != NULL) {
    printTree(node->next_brother, depth);
  }

  // Cetak anak (first_son)
  if (node->first_son != NULL) {
    printTree(node->first_son, depth + 1);
  }
}