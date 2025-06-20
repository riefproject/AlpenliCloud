#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdbool.h>

/**
 * @file linked.h
 * @brief Linked list data structure implementation with generic data support
 * @author Maulana Ishak
 * @date 2025-04-06
 * @license MIT License
 */

#define infotype void*  /**< Generic data type for linked list nodes */

 /*
 ====================================================================
     DATA STRUCTURES
 ====================================================================
 */

 /**
  * @brief Node structure for linked list
  *
  * Represents a single node in the linked list containing data
  * and a pointer to the next node in the sequence.
  */
typedef struct Node {
    infotype data;        /**< Generic data stored in the node */
    struct Node* next;    /**< Pointer to the next node in the list */
} Node;

/**
 * @brief Linked list structure
 *
 * Container structure for managing linked list with head pointer
 * to track the beginning of the list.
 */
typedef struct LinkedList {
    Node* head;           /**< Pointer to the first node in the list */
} LinkedList;

/*
====================================================================
    UTILITY FUNCTIONS
====================================================================
*/

/**
 * @brief Initializes an empty linked list
 *
 * Sets up the linked list structure with head pointer set to NULL,
 * creating an empty list ready for operations.
 *
 * @param[out] list Pointer to linked list to be initialized
 *
 * @pre list must be a valid pointer to LinkedList structure
 * @post List is initialized with head set to NULL (empty state)
 *
 * @note Empty list is represented by NULL head pointer
 * @warning Ensure list pointer is valid before calling this function
 *
 * @see insert_first() for adding first element
 * @see is_list_empty() for checking list state
 *
 * @author Maulana Ishak
 */
void create_list(LinkedList* list);

/**
 * @brief Creates a new node with given data
 *
 * Allocates memory for a new node and initializes it with the
 * provided data. The next pointer is set to NULL.
 *
 * @param[in] data Data to be stored in the new node
 *
 * @return Node* Pointer to newly created node
 * @retval Node* Valid pointer to new node if successful
 * @retval NULL If memory allocation fails
 *
 * @pre data can be any valid infotype value (including NULL)
 * @post New node is created with data stored and next set to NULL
 * @post Memory is allocated for the new node
 *
 * @note Memory allocation is performed using malloc
 * @note Caller is responsible for freeing the node when no longer needed
 * @warning Check return value for NULL before using the node
 *
 * @see insert_first() for adding node to list
 * @see insert_last() for adding node at end
 *
 * @author Maulana Ishak
 */
Node* create_node(infotype data);

/**
 * @brief Checks if the linked list is empty
 *
 * Determines whether the linked list contains any elements by
 * checking if the head pointer is NULL.
 *
 * @param[in] list Linked list to check for emptiness
 *
 * @return bool List emptiness status
 * @retval true List is empty (no elements)
 * @retval false List contains one or more elements
 *
 * @pre list can have NULL head or valid LinkedList structure
 * @post No modification to list state
 *
 * @note Safe to call with uninitialized list
 *
 * @see create_list() for list initialization
 * @see get_length() for getting exact count
 *
 * @author Maulana Ishak
 */
bool is_list_empty(LinkedList list);

/**
 * @brief Checks if given data exists in the list
 *
 * Searches through the linked list to determine if a node
 * containing the specified data exists.
 *
 * @param[in] list Linked list to search
 * @param[in] data Data to search for in the list
 *
 * @return bool Data existence status
 * @retval true Data found in the list
 * @retval false Data not found or list is empty
 *
 * @pre list can be empty or contain valid nodes
 * @pre data can be any valid infotype value
 * @post No modification to list state
 *
 * @note Uses pointer comparison for data matching
 * @note Returns false for empty list
 *
 * @see insert_first() for adding data to list
 * @see delete_val() for removing specific data
 *
 * @author Maulana Ishak
 */
bool is_exist(LinkedList list, infotype data);

/**
 * @brief Gets the length of the linked list
 *
 * Traverses the entire linked list and counts the number of nodes
 * to determine the total length.
 *
 * @param[in] list Linked list to count elements
 *
 * @return int Number of elements in the list
 * @retval 0 List is empty
 * @retval >0 Number of nodes in the list
 *
 * @pre list can be empty or contain valid nodes
 * @post No modification to list state
 *
 * @note Traverses entire list for counting
 * @note Returns 0 for empty list
 *
 * @see is_list_empty() for quick emptiness check
 * @see print_list() for displaying all elements
 *
 * @author Maulana Ishak
 */
int get_length(LinkedList list);

/**
 * @brief Destroys the linked list and frees all memory
 *
 * Removes all nodes from the linked list and frees all allocated
 * memory. The list is reset to empty state after destruction.
 *
 * @param[in,out] list Pointer to linked list to be destroyed
 *
 * @pre list must be a valid pointer to LinkedList structure
 * @post All list nodes are removed and memory is freed
 * @post List is reset to empty state (head set to NULL)
 * @post All allocated memory for nodes is released
 *
 * @note Safe to call on empty list
 * @note List can be reused after destruction by calling create_list()
 * @warning Ensure no references to list nodes remain after destruction
 *
 * @see create_list() for list reinitialization
 * @see delete_first() for removing individual elements
 *
 * @author Maulana Ishak
 */
void destroy_list(LinkedList* list);

/**
 * @brief Prints all elements in the linked list
 *
 * Traverses the linked list and prints all elements in forward order
 * from head to tail. Useful for debugging and visualization.
 *
 * @param[in] list Linked list to print elements from
 *
 * @pre list can be empty or contain valid nodes
 * @post No modification to list state
 * @post Elements are printed to standard output in order
 *
 * @note Safe to call with empty list (prints empty list message)
 * @note Does not modify list contents
 * @note Output format depends on infotype data representation
 *
 * @see get_length() for getting element count
 * @see is_list_empty() for checking list state
 *
 * @author Maulana Ishak
 */
void print_list(LinkedList list);

/*
====================================================================
    INSERT OPERATIONS
====================================================================
*/

/**
 * @brief Inserts a new node at the beginning of the list
 *
 * Creates a new node with the given data and inserts it at the
 * beginning of the linked list. The new node becomes the new head.
 *
 * @param[in,out] list Pointer to linked list to insert into
 * @param[in] data Data to be inserted at the beginning
 *
 * @pre list must be a valid pointer to LinkedList structure
 * @pre data can be any valid infotype value
 * @post New node is created and inserted at the beginning
 * @post List size increases by one
 * @post New node becomes the head of the list
 * @post Previous head (if any) becomes second element
 *
 * @note Memory allocation is performed for new node
 * @note O(1) time complexity operation
 * @warning Ensure sufficient memory is available for new node
 *
 * @see insert_last() for inserting at end
 * @see insert_at() for inserting at specific position
 * @see create_node() for node creation details
 *
 * @author Maulana Ishak
 */
void insert_first(LinkedList* list, infotype data);

/**
 * @brief Inserts a new node at the end of the list
 *
 * Creates a new node with the given data and appends it to the
 * end of the linked list. Traverses to find the last node.
 *
 * @param[in,out] list Pointer to linked list to insert into
 * @param[in] data Data to be inserted at the end
 *
 * @pre list must be a valid pointer to LinkedList structure
 * @pre data can be any valid infotype value
 * @post New node is created and inserted at the end
 * @post List size increases by one
 * @post New node becomes the last element
 * @post If list was empty, new node becomes head
 *
 * @note Requires traversal to find end of list
 * @note O(n) time complexity operation
 * @warning Ensure sufficient memory is available for new node
 *
 * @see insert_first() for inserting at beginning
 * @see insert_at() for inserting at specific position
 *
 * @author Maulana Ishak
 */
void insert_last(LinkedList* list, infotype data);

/**
 * @brief Inserts a new node at the specified position
 *
 * Creates a new node with the given data and inserts it at the
 * specified position in the linked list. Position is 0-indexed.
 *
 * @param[in,out] list Pointer to linked list to insert into
 * @param[in] position 0-based position where to insert the node
 * @param[in] data Data to be inserted at the position
 *
 * @pre list must be a valid pointer to LinkedList structure
 * @pre position must be non-negative and <= list length
 * @pre data can be any valid infotype value
 * @post New node is created and inserted at specified position
 * @post List size increases by one
 * @post Nodes after position are shifted one position forward
 * @post If position is 0, equivalent to insert_first()
 * @post If position >= length, equivalent to insert_last()
 *
 * @note Position validation is performed
 * @note O(n) time complexity operation
 * @warning Invalid position values are handled gracefully
 *
 * @see insert_first() for inserting at beginning
 * @see insert_last() for inserting at end
 * @see get_length() for getting current list size
 *
 * @author Maulana Ishak
 */
void insert_at(LinkedList* list, int position, infotype data);

/*
====================================================================
    DELETE OPERATIONS
====================================================================
*/

/**
 * @brief Deletes the first node from the list
 *
 * Removes the first node from the linked list and returns its data
 * through the temp parameter. Updates head to point to second node.
 *
 * @param[in,out] list Pointer to linked list to delete from
 * @param[out] temp Pointer to store data from deleted node
 *
 * @pre list must be a valid pointer to LinkedList structure
 * @pre temp must be a valid pointer to infotype variable
 * @post First node is removed from list if not empty
 * @post Data from deleted node is stored in temp
 * @post List size decreases by one if not empty
 * @post Head pointer is updated to second node
 * @post Memory for deleted node is freed
 *
 * @note No operation performed if list is empty
 * @note O(1) time complexity operation
 * @note temp parameter receives NULL if list is empty
 *
 * @see delete_last() for deleting last element
 * @see delete_at() for deleting at specific position
 * @see insert_first() for inverse operation
 *
 * @author Maulana Ishak
 */
void delete_first(LinkedList* list, infotype* temp);

/**
 * @brief Deletes the last node from the list
 *
 * Removes the last node from the linked list and returns its data
 * through the temp parameter. Traverses to find the last node.
 *
 * @param[in,out] list Pointer to linked list to delete from
 * @param[out] temp Pointer to store data from deleted node
 *
 * @pre list must be a valid pointer to LinkedList structure
 * @pre temp must be a valid pointer to infotype variable
 * @post Last node is removed from list if not empty
 * @post Data from deleted node is stored in temp
 * @post List size decreases by one if not empty
 * @post Previous node becomes new last node
 * @post Memory for deleted node is freed
 *
 * @note No operation performed if list is empty
 * @note O(n) time complexity operation
 * @note temp parameter receives NULL if list is empty
 *
 * @see delete_first() for deleting first element
 * @see delete_at() for deleting at specific position
 * @see insert_last() for inverse operation
 *
 * @author Maulana Ishak
 */
void delete_last(LinkedList* list, infotype* temp);

/**
 * @brief Deletes the first node containing specified data
 *
 * Searches for and removes the first node containing the specified
 * data from the linked list. Returns the data through temp parameter.
 *
 * @param[in,out] list Pointer to linked list to delete from
 * @param[in] data Data to search for and delete
 * @param[out] temp Pointer to store data from deleted node
 *
 * @pre list must be a valid pointer to LinkedList structure
 * @pre data can be any valid infotype value
 * @pre temp must be a valid pointer to infotype variable
 * @post First node with matching data is removed if found
 * @post Data from deleted node is stored in temp
 * @post List size decreases by one if node found
 * @post Node links are properly updated
 * @post Memory for deleted node is freed
 *
 * @note No operation performed if data not found
 * @note Uses pointer comparison for data matching
 * @note temp parameter receives NULL if data not found
 *
 * @see delete_first() for deleting by position
 * @see delete_at() for deleting at specific position
 * @see is_exist() for checking data existence
 *
 * @author Maulana Ishak
 */
void delete_val(LinkedList* list, infotype data, infotype* temp);

/**
 * @brief Deletes the node at the specified position
 *
 * Removes the node at the specified position from the linked list
 * and returns its data through the temp parameter. Position is 0-indexed.
 *
 * @param[in,out] list Pointer to linked list to delete from
 * @param[in] position 0-based position of node to delete
 * @param[out] temp Pointer to store data from deleted node
 *
 * @pre list must be a valid pointer to LinkedList structure
 * @pre position must be non-negative and < list length
 * @pre temp must be a valid pointer to infotype variable
 * @post Node at specified position is removed if position valid
 * @post Data from deleted node is stored in temp
 * @post List size decreases by one if position valid
 * @post Nodes after position are shifted one position backward
 * @post Memory for deleted node is freed
 *
 * @note No operation performed if position is invalid
 * @note Position validation is performed
 * @note temp parameter receives NULL if position invalid
 *
 * @see delete_first() for deleting first element
 * @see delete_last() for deleting last element
 * @see insert_at() for inverse operation
 * @see get_length() for getting valid position range
 *
 * @author Maulana Ishak
 */
void delete_at(LinkedList* list, int position, infotype* temp);

#endif // LINKED_LIST_H