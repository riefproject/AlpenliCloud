#ifndef NBTREE_H
#define NBTREE_H

#include "item.h"

/**
 * @file nbtree.h
 * @brief N-ary tree data structure implementation for hierarchical file system representation
 * @date 2025
 * @license MIT License
 */

 /*
 ====================================================================
     TYPE DEFINITIONS AND DATA STRUCTURES
 ====================================================================
 */

 /**
  * @brief Tree node information type
  *
  * Defines the data type stored in each tree node. Uses Item structure
  * to represent file system entities (files and directories).
  */
typedef Item treeInfotype;

/**
 * @brief Tree node pointer type
 *
 * Pointer to TreeNode structure representing a node in the N-ary tree.
 */
typedef struct TreeNode* Tree;

/**
 * @brief N-ary tree node structure
 *
 * Represents a node in an N-ary tree with support for multiple children
 * and parent relationships. Used for hierarchical file system representation.
 */
typedef struct TreeNode {
  treeInfotype item;     /**< Data stored in this node (file/directory information) */
  Tree next_brother;     /**< Pointer to next sibling node at same level */
  Tree first_son;        /**< Pointer to first child node */
  Tree parent;           /**< Pointer to parent node */
} TreeNode;

/*
====================================================================
    TREE OPERATIONS
====================================================================
*/

/**
 * @brief Creates and initializes an empty tree
 *
 * Initializes the tree pointer to NULL, creating an empty tree
 * ready for node insertion and tree operations.
 *
 * @param[out] tree Pointer to tree to be initialized
 *
 * @pre tree must be a valid pointer to Tree
 * @post Tree is initialized to NULL (empty state)
 *
 * @note Empty tree is represented by NULL pointer
 * @warning Ensure tree pointer is valid before calling this function
 *
 * @see create_node_tree() for creating individual nodes
 * @see insert_node() for adding nodes to tree
 *
 */
void create_tree(Tree* tree);

/**
 * @brief Creates a new tree node with given data
 *
 * Allocates memory for a new tree node and initializes it with the
 * provided data. All pointer fields are set to NULL.
 *
 * @param[in] data Data to be stored in the new node
 *
 * @return Tree Pointer to newly created node
 * @retval Tree Valid pointer to new node if successful
 * @retval NULL If memory allocation fails
 *
 * @pre data must be a valid treeInfotype value
 * @post New node is created with data stored
 * @post All pointer fields (next_brother, first_son, parent) are set to NULL
 * @post Memory is allocated for the new node
 *
 * @note Memory allocation is performed using malloc
 * @note Caller is responsible for freeing the node when no longer needed
 * @warning Check return value for NULL before using the node
 *
 * @see insert_node() for adding node to existing tree
 * @see create_tree() for tree initialization
 *
 */
Tree create_node_tree(treeInfotype data);

/**
 * @brief Inserts a new node as child of specified parent
 *
 * Creates a new node with given data and inserts it as a child of the
 * specified parent node. The new node is added to the end of the sibling list.
 *
 * @param[in] parent Parent node to add child to
 * @param[in] data Data for the new child node
 *
 * @return Tree Pointer to newly created and inserted node
 * @retval Tree Valid pointer to new child node if successful
 * @retval NULL If memory allocation fails or parent is NULL
 *
 * @pre parent can be NULL or valid Tree node
 * @pre data must be a valid treeInfotype value
 * @post New node is created and added as child of parent
 * @post If parent has no children, new node becomes first_son
 * @post If parent has children, new node is added at end of sibling list
 * @post New node's parent pointer is set to parent
 *
 * @note If parent is NULL, creates standalone node without parent relationship
 * @note New node is always added at the end of existing children
 * @warning Ensure parent node remains valid if not NULL
 *
 * @see create_node_tree() for creating standalone nodes
 * @see searchTree() for finding nodes in tree
 *
 */
Tree insert_node(Tree parent, treeInfotype data);

/**
 * @brief Searches for a node with matching data in the tree
 *
 * Performs recursive search through the tree to find a node containing
 * data that matches the specified criteria. Uses depth-first search approach.
 *
 * @param[in] root Root node to start search from
 * @param[in] data Data to search for in the tree
 *
 * @return Tree Pointer to found node or NULL if not found
 * @retval Tree Valid pointer to node containing matching data
 * @retval NULL If data not found or root is NULL
 *
 * @pre root can be NULL or valid Tree node
 * @pre data must be a valid treeInfotype value for comparison
 * @post No modification to tree structure
 * @post Returns first node found with matching data
 *
 * @note Uses depth-first search algorithm
 * @note Comparison depends on treeInfotype comparison implementation
 * @note Returns first match found during traversal
 *
 * @see insert_node() for adding nodes to search
 * @see printTree() for tree visualization
 *
 */
Tree searchTree(Tree root, treeInfotype data);

/**
 * @brief Prints indentation for tree visualization
 *
 * Helper function that prints spaces for proper tree indentation
 * based on the specified depth level.
 *
 * @param[in] depth Indentation depth level
 *
 * @pre depth must be non-negative integer
 * @post Spaces are printed to standard output for indentation
 *
 * @note Each depth level adds fixed number of spaces
 * @note Used internally by printTree() for formatting
 *
 * @see printTree() for complete tree printing
 *
 */
void printIndent(int depth);

/**
 * @brief Prints tree structure with hierarchical formatting
 *
 * Recursively prints the entire tree structure with proper indentation
 * to visualize the hierarchical relationships between nodes.
 *
 * @param[in] node Root node to start printing from
 * @param[in] depth Current depth level for indentation
 *
 * @pre node can be NULL or valid Tree node
 * @pre depth must be non-negative integer
 * @post Tree structure is printed to standard output with indentation
 * @post Each level is properly indented based on depth
 * @post All children and siblings are printed recursively
 *
 * @note Uses depth-first traversal for printing
 * @note Safe to call with NULL node (no output)
 * @note Indentation helps visualize tree structure
 *
 * @see printIndent() for indentation formatting
 * @see searchTree() for finding specific nodes
 *
 */
void printTree(Tree node, int depth);

#endif // NBTREE_H