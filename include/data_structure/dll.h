#ifndef DLL_H
#define DLL_H

/**
 * @file dll.h
 * @brief Doubly linked list data structure implementation with generic data support
 * @author Arief F-sa Wijaya
 * @date 2025
 * @license MIT License
 */

#ifndef ALOCATE_SHORTCUTS
#define ALOCATE_SHORTCUTS
#define new(type) (type*)malloc(sizeof(type))  /**< Memory allocation shortcut macro */
#define delete(ptr) do { free(ptr); ptr = NULL; } while (0)  /**< Memory deallocation shortcut macro */
#endif

#include <stdbool.h>
#include <stdlib.h>

 /*
 ====================================================================
     FORWARD DECLARATIONS AND DATA STRUCTURES
 ====================================================================
 */

typedef struct SingleLinkedList SingleLinkedList;
typedef struct DLLNode DLLNode;

/**
 * @brief Doubly linked list node structure
 *
 * Represents a node in a doubly linked list containing data and
 * pointers to both next and previous nodes in the sequence.
 */
typedef struct DLLNode {
    void* data;              /**< Generic data stored in the node */
    struct DLLNode* next;    /**< Pointer to the next node in the list */
    struct DLLNode* prev;    /**< Pointer to the previous node in the list */
} DLLNode;

/**
 * @brief Doubly linked list structure
 *
 * Container structure for managing doubly linked list with head, tail
 * pointers and size tracking for efficient operations.
 */
typedef struct DoublyLinkedList {
    DLLNode* head;    /**< Pointer to the first node in the list */
    DLLNode* tail;    /**< Pointer to the last node in the list */
    int size;         /**< Current number of nodes in the list */
} DoublyLinkedList;

/*
====================================================================
    CONSTRUCTOR FUNCTIONS
====================================================================
*/

/**
 * @brief Creates and initializes a new doubly linked list
 *
 * Allocates memory for a new DoublyLinkedList structure and initializes
 * all fields to default values (NULL pointers and zero size).
 *
 * @return DoublyLinkedList* Pointer to newly created list
 * @retval DoublyLinkedList* Valid pointer to new list if successful
 * @retval NULL If memory allocation fails
 *
 * @post New list is created with head and tail set to NULL
 * @post Size is initialized to 0
 * @post Memory is allocated for the list structure
 *
 * @note Memory allocation is performed using malloc
 * @note Caller is responsible for freeing the list with DLL_freeList()
 * @warning Check return value for NULL before using the list
 *
 * @see DLL_freeList() for proper cleanup
 * @see createDLLNode() for creating individual nodes
 *
 * @author Arief F-sa Wijaya
 */
DoublyLinkedList* createDoublyLinkedList();

/**
 * @brief Creates a new doubly linked list node with given data
 *
 * Allocates memory for a new DLLNode and initializes it with the
 * provided data. Both next and prev pointers are set to NULL.
 *
 * @param[in] data Data to be stored in the new node
 *
 * @return DLLNode* Pointer to newly created node
 * @retval DLLNode* Valid pointer to new node if successful
 * @retval NULL If memory allocation fails
 *
 * @pre data can be any valid pointer (including NULL)
 * @post New node is created with data stored
 * @post next and prev pointers are set to NULL
 * @post Memory is allocated for the new node
 *
 * @note Memory allocation is performed using malloc
 * @note Caller is responsible for proper integration into list
 * @warning Check return value for NULL before using the node
 *
 * @see createDoublyLinkedList() for creating list container
 * @see DLL_freeNode() for node cleanup
 *
 * @author Arief F-sa Wijaya
 */
DLLNode* createDLLNode(void* data);

/*
====================================================================
    GETTER FUNCTIONS
====================================================================
*/

/**
 * @brief Gets the data stored in a doubly linked list node
 *
 * Returns the data pointer stored in the specified node.
 * Safe to call with NULL node (returns NULL).
 *
 * @param[in] node Node to get data from
 *
 * @return void* Data stored in the node
 * @retval void* Valid data pointer if node is valid
 * @retval NULL If node is NULL
 *
 * @pre node can be NULL or valid DLLNode pointer
 * @post No modification to node state
 *
 * @note Safe to call with NULL node
 * @note Returns actual data pointer, not a copy
 *
 * @see DLL_setNodeData() for setting node data
 *
 * @author Arief F-sa Wijaya
 */
void* DLL_getNodeData(DLLNode* node);

/**
 * @brief Gets the next node in the doubly linked list
 *
 * Returns the pointer to the next node in the list sequence.
 * Safe to call with NULL node (returns NULL).
 *
 * @param[in] node Node to get next node from
 *
 * @return DLLNode* Pointer to next node
 * @retval DLLNode* Valid node pointer if next exists
 * @retval NULL If node is NULL or no next node exists
 *
 * @pre node can be NULL or valid DLLNode pointer
 * @post No modification to node state
 *
 * @note Safe to call with NULL node
 * @note Returns NULL if node is the last node in list
 *
 * @see DLL_getPrevNode() for getting previous node
 * @see DLL_setNextNode() for setting next node
 *
 * @author Arief F-sa Wijaya
 */
DLLNode* DLL_getNextNode(DLLNode* node);

/**
 * @brief Gets the previous node in the doubly linked list
 *
 * Returns the pointer to the previous node in the list sequence.
 * Safe to call with NULL node (returns NULL).
 *
 * @param[in] node Node to get previous node from
 *
 * @return DLLNode* Pointer to previous node
 * @retval DLLNode* Valid node pointer if previous exists
 * @retval NULL If node is NULL or no previous node exists
 *
 * @pre node can be NULL or valid DLLNode pointer
 * @post No modification to node state
 *
 * @note Safe to call with NULL node
 * @note Returns NULL if node is the first node in list
 *
 * @see DLL_getNextNode() for getting next node
 * @see DLL_setPrevNode() for setting previous node
 *
 * @author Arief F-sa Wijaya
 */
DLLNode* DLL_getPrevNode(DLLNode* node);

/*
====================================================================
    SETTER FUNCTIONS
====================================================================
*/

/**
 * @brief Sets the data stored in a doubly linked list node
 *
 * Updates the data pointer stored in the specified node.
 * Safe to call with NULL node (no operation performed).
 *
 * @param[in,out] node Node to set data for
 * @param[in] data New data to store in the node
 *
 * @pre node can be NULL or valid DLLNode pointer
 * @pre data can be any valid pointer (including NULL)
 * @post Node data is updated to new value if node is valid
 *
 * @note Safe to call with NULL node
 * @note Previous data pointer is overwritten (not freed)
 * @warning Caller responsible for managing data memory
 *
 * @see DLL_getNodeData() for getting node data
 *
 * @author Arief F-sa Wijaya
 */
void DLL_setNodeData(DLLNode* node, void* data);

/**
 * @brief Sets the next node pointer in a doubly linked list node
 *
 * Updates the next pointer of the specified node to point to
 * the provided next node.
 *
 * @param[in,out] node Node to set next pointer for
 * @param[in] next New next node pointer
 *
 * @pre node can be NULL or valid DLLNode pointer
 * @pre next can be NULL or valid DLLNode pointer
 * @post Node next pointer is updated if node is valid
 *
 * @note Safe to call with NULL node
 * @note Does not update prev pointer of next node
 * @warning Ensure proper bidirectional linking manually
 *
 * @see DLL_getNextNode() for getting next node
 * @see DLL_setPrevNode() for setting previous node
 *
 * @author Arief F-sa Wijaya
 */
void DLL_setNextNode(DLLNode* node, DLLNode* next);

/**
 * @brief Sets the previous node pointer in a doubly linked list node
 *
 * Updates the prev pointer of the specified node to point to
 * the provided previous node.
 *
 * @param[in,out] node Node to set previous pointer for
 * @param[in] prev New previous node pointer
 *
 * @pre node can be NULL or valid DLLNode pointer
 * @pre prev can be NULL or valid DLLNode pointer
 * @post Node prev pointer is updated if node is valid
 *
 * @note Safe to call with NULL node
 * @note Does not update next pointer of prev node
 * @warning Ensure proper bidirectional linking manually
 *
 * @see DLL_getPrevNode() for getting previous node
 * @see DLL_setNextNode() for setting next node
 *
 * @author Arief F-sa Wijaya
 */
void DLL_setPrevNode(DLLNode* node, DLLNode* prev);

/*
====================================================================
    INSERT OPERATIONS
====================================================================
*/

/**
 * @brief Inserts a new node at the front of the doubly linked list
 *
 * Creates a new node with the given data and inserts it at the
 * beginning of the list. Updates head pointer and maintains size.
 *
 * @param[in,out] list Doubly linked list to insert into
 * @param[in] data Data to be inserted at the front
 *
 * @pre list must be a valid pointer to DoublyLinkedList structure
 * @pre data can be any valid pointer (including NULL)
 * @post New node is created and inserted at the front
 * @post List size increases by one
 * @post New node becomes the head of the list
 * @post Previous head (if any) becomes second element
 * @post Bidirectional links are properly maintained
 *
 * @note Memory allocation is performed for new node
 * @note O(1) time complexity operation
 * @warning Ensure sufficient memory is available for new node
 *
 * @see DLL_insertBack() for inserting at end
 * @see DLL_insertNode() for inserting at specific position
 *
 * @author Arief F-sa Wijaya
 */
void DLL_insertFront(DoublyLinkedList* list, void* data);

/**
 * @brief Inserts a new node at the back of the doubly linked list
 *
 * Creates a new node with the given data and appends it to the
 * end of the list. Updates tail pointer and maintains size.
 *
 * @param[in,out] list Doubly linked list to insert into
 * @param[in] data Data to be inserted at the back
 *
 * @pre list must be a valid pointer to DoublyLinkedList structure
 * @pre data can be any valid pointer (including NULL)
 * @post New node is created and inserted at the back
 * @post List size increases by one
 * @post New node becomes the tail of the list
 * @post Previous tail (if any) links to new node
 * @post Bidirectional links are properly maintained
 *
 * @note Memory allocation is performed for new node
 * @note O(1) time complexity operation due to tail pointer
 * @warning Ensure sufficient memory is available for new node
 *
 * @see DLL_insertFront() for inserting at beginning
 * @see DLL_insertNode() for inserting at specific position
 *
 * @author Arief F-sa Wijaya
 */
void DLL_insertBack(DoublyLinkedList* list, void* data);

/**
 * @brief Inserts a new node at the specified position
 *
 * Creates a new node with the given data and inserts it at the
 * specified position in the list. Position is 0-indexed.
 *
 * @param[in,out] list Doubly linked list to insert into
 * @param[in] data Data to be inserted
 * @param[in] pos 0-based position where to insert the node
 *
 * @pre list must be a valid pointer to DoublyLinkedList structure
 * @pre data can be any valid pointer (including NULL)
 * @pre pos must be non-negative and <= list size
 * @post New node is created and inserted at specified position
 * @post List size increases by one
 * @post Nodes after position are shifted one position forward
 * @post Bidirectional links are properly maintained
 * @post If pos is 0, equivalent to DLL_insertFront()
 * @post If pos >= size, equivalent to DLL_insertBack()
 *
 * @note Position validation is performed
 * @note Time complexity varies based on position
 * @warning Invalid position values are handled gracefully
 *
 * @see DLL_insertFront() for inserting at beginning
 * @see DLL_insertBack() for inserting at end
 *
 * @author Arief F-sa Wijaya
 */
void DLL_insertNode(DoublyLinkedList* list, void* data, int pos);

/*
====================================================================
    REMOVE OPERATIONS
====================================================================
*/

/**
 * @brief Removes the front node from the doubly linked list
 *
 * Removes the first node from the list and stores its reference
 * in the provided node parameter for potential data retrieval.
 *
 * @param[in,out] list Doubly linked list to remove from
 * @param[out] node Pointer to store reference to removed node
 *
 * @pre list must be a valid pointer to DoublyLinkedList structure
 * @pre node must be a valid pointer to DLLNode pointer
 * @post Front node is removed from list if not empty
 * @post Reference to removed node is stored in node parameter
 * @post List size decreases by one if not empty
 * @post Head pointer is updated to second node
 * @post Bidirectional links are properly maintained
 *
 * @note No operation performed if list is empty
 * @note O(1) time complexity operation
 * @note node parameter receives NULL if list is empty
 * @warning Caller responsible for freeing removed node
 *
 * @see DLL_removeBack() for removing last element
 * @see DLL_removeNode() for removing at specific position
 * @see DLL_freeNode() for proper node cleanup
 *
 * @author Arief F-sa Wijaya
 */
void DLL_removeFront(DoublyLinkedList* list, DLLNode* node);

/**
 * @brief Removes the back node from the doubly linked list
 *
 * Removes the last node from the list and stores its reference
 * in the provided node parameter for potential data retrieval.
 *
 * @param[in,out] list Doubly linked list to remove from
 * @param[out] node Pointer to store reference to removed node
 *
 * @pre list must be a valid pointer to DoublyLinkedList structure
 * @pre node must be a valid pointer to DLLNode pointer
 * @post Back node is removed from list if not empty
 * @post Reference to removed node is stored in node parameter
 * @post List size decreases by one if not empty
 * @post Tail pointer is updated to second-to-last node
 * @post Bidirectional links are properly maintained
 *
 * @note No operation performed if list is empty
 * @note O(1) time complexity operation due to tail pointer
 * @note node parameter receives NULL if list is empty
 * @warning Caller responsible for freeing removed node
 *
 * @see DLL_removeFront() for removing first element
 * @see DLL_removeNode() for removing at specific position
 * @see DLL_freeNode() for proper node cleanup
 *
 * @author Arief F-sa Wijaya
 */
void DLL_removeBack(DoublyLinkedList* list, DLLNode* node);

/**
 * @brief Removes the node at the specified position
 *
 * Removes the node at the specified position from the list and
 * stores its reference in the provided node parameter.
 *
 * @param[in,out] list Doubly linked list to remove from
 * @param[out] node Pointer to store reference to removed node
 * @param[in] pos 0-based position of node to remove
 *
 * @pre list must be a valid pointer to DoublyLinkedList structure
 * @pre node must be a valid pointer to DLLNode pointer
 * @pre pos must be non-negative and < list size
 * @post Node at specified position is removed if position valid
 * @post Reference to removed node is stored in node parameter
 * @post List size decreases by one if position valid
 * @post Nodes after position are shifted one position backward
 * @post Bidirectional links are properly maintained
 *
 * @note No operation performed if position is invalid
 * @note Position validation is performed
 * @note node parameter receives NULL if position invalid
 * @warning Caller responsible for freeing removed node
 *
 * @see DLL_removeFront() for removing first element
 * @see DLL_removeBack() for removing last element
 * @see DLL_freeNode() for proper node cleanup
 *
 * @author Arief F-sa Wijaya
 */
void DLL_removeNode(DoublyLinkedList* list, DLLNode* node, int pos);

/*
====================================================================
    GET OPERATIONS
====================================================================
*/

/**
 * @brief Gets the data from the front node of the list
 *
 * Returns the data stored in the first node of the list
 * without removing the node from the list.
 *
 * @param[in] list Doubly linked list to get data from
 *
 * @return void* Data from front node
 * @retval void* Valid data pointer if list is not empty
 * @retval NULL If list is empty or NULL
 *
 * @pre list can be NULL or valid DoublyLinkedList pointer
 * @post No modification to list state
 *
 * @note Safe to call with empty list (returns NULL)
 * @note Returns actual data pointer, not a copy
 *
 * @see DLL_getBack() for getting back data
 * @see DLL_getNode() for getting data at specific position
 *
 * @author Arief F-sa Wijaya
 */
void* DLL_getFront(DoublyLinkedList* list);

/**
 * @brief Gets the data from the back node of the list
 *
 * Returns the data stored in the last node of the list
 * without removing the node from the list.
 *
 * @param[in] list Doubly linked list to get data from
 *
 * @return void* Data from back node
 * @retval void* Valid data pointer if list is not empty
 * @retval NULL If list is empty or NULL
 *
 * @pre list can be NULL or valid DoublyLinkedList pointer
 * @post No modification to list state
 *
 * @note Safe to call with empty list (returns NULL)
 * @note O(1) time complexity due to tail pointer
 * @note Returns actual data pointer, not a copy
 *
 * @see DLL_getFront() for getting front data
 * @see DLL_getNode() for getting data at specific position
 *
 * @author Arief F-sa Wijaya
 */
void* DLL_getBack(DoublyLinkedList* list);

/**
 * @brief Gets the data from the node at specified position
 *
 * Returns the data stored in the node at the specified position
 * without removing the node from the list. Position is 0-indexed.
 *
 * @param[in] list Doubly linked list to get data from
 * @param[in] pos 0-based position of node to get data from
 *
 * @return void* Data from node at position
 * @retval void* Valid data pointer if position is valid
 * @retval NULL If position is invalid or list is empty
 *
 * @pre list can be NULL or valid DoublyLinkedList pointer
 * @pre pos must be non-negative and < list size
 * @post No modification to list state
 *
 * @note Position validation is performed
 * @note Returns NULL for invalid positions
 * @note Time complexity varies based on position
 *
 * @see DLL_getFront() for getting front data
 * @see DLL_getBack() for getting back data
 *
 * @author Arief F-sa Wijaya
 */
void* DLL_getNode(DoublyLinkedList* list, int pos);

/*
====================================================================
    MEMORY MANAGEMENT FUNCTIONS
====================================================================
*/

/**
 * @brief Clears all nodes from the doubly linked list
 *
 * Removes and frees all nodes from the list, leaving it in an
 * empty but usable state. The list structure itself is preserved.
 *
 * @param[in,out] list Doubly linked list to clear
 *
 * @pre list must be a valid pointer to DoublyLinkedList structure
 * @post All nodes are removed and memory is freed
 * @post List is reset to empty state (head/tail NULL, size 0)
 * @post List structure remains valid for further operations
 *
 * @note Safe to call on empty list
 * @note List can be reused immediately after clearing
 * @warning Ensure no external references to list nodes remain
 *
 * @see DLL_freeList() for complete list destruction
 * @see createDoublyLinkedList() for list creation
 *
 * @author Arief F-sa Wijaya
 */
void DLL_clearList(DoublyLinkedList* list);

/**
 * @brief Frees the entire doubly linked list and its structure
 *
 * Removes all nodes from the list, frees their memory, and then
 * frees the list structure itself. The list pointer becomes invalid.
 *
 * @param[in,out] list Doubly linked list to free completely
 *
 * @pre list must be a valid pointer to DoublyLinkedList structure
 * @post All nodes are removed and memory is freed
 * @post List structure itself is freed
 * @post All allocated memory is released
 *
 * @note List pointer becomes invalid after this call
 * @note Cannot be reused without creating a new list
 * @warning Do not use list pointer after calling this function
 *
 * @see DLL_clearList() for clearing without destroying structure
 * @see createDoublyLinkedList() for creating new list
 *
 * @author Arief F-sa Wijaya
 */
void DLL_freeList(DoublyLinkedList* list);

/**
 * @brief Frees a single doubly linked list node
 *
 * Frees the memory allocated for a single node. The node
 * should already be removed from any list structure.
 *
 * @param[in,out] node Node to be freed
 *
 * @pre node can be NULL or valid DLLNode pointer
 * @post Node memory is freed if node is valid
 * @post Node pointer becomes invalid
 *
 * @note Safe to call with NULL node
 * @note Does not free the data pointed to by node->data
 * @warning Ensure node is not part of any list before freeing
 * @warning Caller responsible for freeing node data separately
 *
 * @see DLL_removeFront() for removing nodes from list
 * @see createDLLNode() for node creation
 *
 * @author Arief F-sa Wijaya
 */
void DLL_freeNode(DLLNode* node);

/*
====================================================================
    PRINT OPERATIONS
====================================================================
*/

/**
 * @brief Prints all elements in the doubly linked list forward
 *
 * Traverses the list from head to tail and prints all elements
 * using the provided print function. Useful for debugging and visualization.
 *
 * @param[in] list Doubly linked list to print
 * @param[in] printFunc Function pointer for printing individual data elements
 *
 * @pre list can be NULL or valid DoublyLinkedList pointer
 * @pre printFunc must be a valid function pointer that accepts void*
 * @post No modification to list state
 * @post Elements are printed to standard output in forward order
 *
 * @note Safe to call with empty list (prints empty list message)
 * @note Does not modify list contents
 * @note Print format depends on provided print function
 *
 * @see DLL_printListReverse() for reverse order printing
 *
 * @author Arief F-sa Wijaya
 */
void DLL_printList(DoublyLinkedList* list, void (*printFunc)(void*));

/**
 * @brief Prints all elements in the doubly linked list in reverse order
 *
 * Traverses the list from tail to head and prints all elements
 * using the provided print function. Demonstrates bidirectional capability.
 *
 * @param[in] list Doubly linked list to print in reverse
 * @param[in] printFunc Function pointer for printing individual data elements
 *
 * @pre list can be NULL or valid DoublyLinkedList pointer
 * @pre printFunc must be a valid function pointer that accepts void*
 * @post No modification to list state
 * @post Elements are printed to standard output in reverse order
 *
 * @note Safe to call with empty list (prints empty list message)
 * @note Does not modify list contents
 * @note Demonstrates advantage of doubly linked structure
 *
 * @see DLL_printList() for forward order printing
 *
 * @author Arief F-sa Wijaya
 */
void DLL_printListReverse(DoublyLinkedList* list, void (*printFunc)(void*));

#endif // DLL_H