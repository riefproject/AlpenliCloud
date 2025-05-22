#include <stdio.h>
#include <stdlib.h>
#include "nbtree.h"
#include <string.h>

void create_tree(Tree *tree)
{
  *tree = NULL;
}

Tree create_node_tree(treeInfotype data)
{
  TreeNode *newNode = (TreeNode *)malloc(sizeof(TreeNode));
  if (newNode == NULL)
    return NULL;

  newNode->item = data;
  newNode->next_brother = NULL;
  newNode->first_son = NULL;
  newNode->parent = NULL;
  return newNode;
}

Tree insert_node(Tree parent, treeInfotype data)
{
  TreeNode *child = create_node_tree(data);
  if (parent == NULL || child == NULL)
    return;

  printf("%s=================\n", parent->item.name);

  if (parent->first_son == NULL)
  {
    parent->first_son = child;
  }
  else
  {
    Tree sibling = parent->first_son;
    while (sibling->next_brother != NULL)
      sibling = sibling->next_brother;
    sibling->next_brother = child;
  }
}

Tree searchTree(Tree root, treeInfotype item)
{
  // Kalau tree kosoong, return null
  if (root == NULL)
    return NULL;
  // kalau root sama dengan item yang dicari, return root
  if (strcmp(root->item.name, item.name) == 0 && strcmp(root->item.path, item.path) == 0)
    return root;
  // cari di first_son
  Tree found = searchTree(root->first_son, item);
  // kalau ketemu, return found
  if (found != NULL)
    return found;
  // cari di next_brother
  return searchTree(root->next_brother, item);
}

// Fungsi bantu untuk mencetak indentasi
void printIndent(int depth)
{
  for (int i = 0; i < depth; i++)
  {
    printf("  "); // dua spasi per level
  }
}

// Fungsi rekursif untuk mencetak pohon
void printTree(Tree node, int depth)
{
  if (node == NULL)
    return;

  // Cetak node saat ini
  printIndent(depth);

  char *typeStr = node->item.type == ITEM_FOLDER ? "(folder)" : "(file)";

  // Format waktu
  char updated_at_str[20];
  strftime(updated_at_str, sizeof(updated_at_str), "%Y-%m-%d", localtime(&node->item.updated_at));

  printf("%s %s (size: %ld, updated: %s)\n",
         typeStr,
         node->item.name,
         node->item.size,
         updated_at_str);

  // Cetak anak (first_son)
  if (node->first_son != NULL)
  {
    printTree(node->first_son, depth + 1);
  }

  // Cetak saudara (next_brother)
  if (node->next_brother != NULL)
  {
    printTree(node->next_brother, depth);
  }
}