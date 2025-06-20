#ifndef QUEUE_H
#define QUEUE_H

#include "linked.h"
#include "item.h"

/**
 * @file queue.h
 * @brief Queue data structure implementation using linked list
 * @author Maulana Ishak
 * @date 2025-04-09
 * @license MIT License
 */

 /*
 ====================================================================
     DATA STRUCTURES
 ====================================================================
 */

 /**
  * @brief Queue data structure
  *
  * Queue implementation using linked list with front and rear pointers.
  * Follows First-In-First-Out (FIFO) principle for data storage and retrieval.
  */
typedef struct Queue {
  Node* front;  /**< Pointer to front node of the queue */
  Node* rear;   /**< Pointer to rear node of the queue */
} Queue;

/*
====================================================================
    QUEUE OPERATIONS
====================================================================
*/

/**
 * @brief Creates and initializes an empty queue
 *
 * Initializes the queue structure with front and rear pointers set to NULL,
 * creating an empty queue ready for enqueue and dequeue operations.
 *
 * @param[out] queue Pointer to queue structure to be initialized
 *
 * @pre queue must be a valid pointer to Queue structure
 * @post Queue is initialized with front and rear set to NULL (empty state)
 *
 * @note Queue is implemented using linked list with NULL pointers indicating empty queue
 * @warning Ensure queue pointer is valid before calling this function
 *
 * @see enqueue() for adding elements to queue
 * @see dequeue() for removing elements from queue
 *
 * @author Maulana Ishak
 */
void create_queue(Queue* queue);

/**
 * @brief Checks if the queue is empty
 *
 * Determines whether the queue contains any elements by checking
 * if the front pointer is NULL.
 *
 * @param[in] queue Queue to check for emptiness
 *
 * @return bool Queue emptiness status
 * @retval true Queue is empty (no elements)
 * @retval false Queue contains one or more elements
 *
 * @pre queue can have NULL front pointer or valid Queue structure
 * @post No modification to queue state
 *
 * @note Safe to call with uninitialized queue (returns true if front is NULL)
 *
 * @see create_queue() for queue initialization
 *
 * @author Maulana Ishak
 */
bool is_queue_empty(Queue queue);

/**
 * @brief Enqueues an element to the rear of the queue
 *
 * Adds a new element to the rear of the queue using linked list insertion.
 * The new element becomes the new rear of the queue.
 *
 * @param[in,out] queue Pointer to queue to add element to
 * @param[in] data Data to be enqueued
 *
 * @pre queue must be a valid pointer to Queue structure
 * @pre data must be a valid infotype value
 * @post New element is added to rear of queue
 * @post Queue size increases by one
 * @post If queue was empty, both front and rear point to new element
 * @post If queue was not empty, rear pointer is updated to new element
 *
 * @note Memory allocation is performed for new queue element
 * @note Queue can grow indefinitely (limited only by available memory)
 * @warning Ensure sufficient memory is available for queue growth
 *
 * @see dequeue() for removing elements from queue
 * @see is_queue_empty() for checking queue state
 *
 * @author Maulana Ishak
 */
void enqueue(Queue* queue, infotype data);

/**
 * @brief Dequeues and returns the front element from the queue
 *
 * Removes and returns the front element from the queue. The element
 * is removed from the queue and its data is returned to caller.
 *
 * @param[in,out] queue Pointer to queue to remove element from
 *
 * @return infotype Data from dequeued element
 * @retval infotype Valid data if queue is not empty
 * @retval Default infotype value if queue is empty
 *
 * @pre queue must be a valid pointer to Queue structure
 * @post Front element is removed from queue if not empty
 * @post Queue size decreases by one if not empty
 * @post Front pointer is updated to next element
 * @post If queue becomes empty, both front and rear are set to NULL
 * @post Memory for dequeued node is freed
 *
 * @note Returns default infotype value for empty queue
 * @note Updates both front and rear pointers when queue becomes empty
 * @warning Check queue emptiness before calling to avoid undefined behavior
 *
 * @see enqueue() for adding elements to queue
 * @see is_queue_empty() for checking queue state
 *
 * @author Maulana Ishak
 */
infotype dequeue(Queue* queue);

/**
 * @brief Destroys the queue and frees all allocated memory
 *
 * Removes all elements from the queue and frees all allocated memory.
 * The queue structure is reset to empty state after destruction.
 *
 * @param[in,out] queue Pointer to queue to be destroyed
 *
 * @pre queue must be a valid pointer to Queue structure
 * @post All queue elements are removed and memory is freed
 * @post Queue is reset to empty state (front and rear set to NULL)
 * @post All allocated memory for nodes is released
 *
 * @note Safe to call on empty queue
 * @note Queue can be reused after destruction by calling create_queue()
 * @warning Ensure no references to queue elements remain after destruction
 *
 * @see create_queue() for queue reinitialization
 * @see clear_queue() for alternative cleanup method
 *
 * @author Maulana Ishak
 */
void destroy_queue(Queue* queue);

/**
 * @brief Prints all elements in the queue from front to rear
 *
 * Displays all elements in the queue in order from front to rear
 * without modifying the queue structure. Useful for debugging and visualization.
 *
 * @param[in] queue Queue to print elements from
 *
 * @pre queue can be empty or contain valid elements
 * @post No modification to queue state
 * @post Elements are printed to standard output from front to rear
 *
 * @note Safe to call with empty queue (prints empty queue message)
 * @note Does not modify queue contents
 * @note Output format depends on infotype data structure
 *
 * @see is_queue_empty() for checking queue state
 *
 * @author Maulana Ishak
 */
void print_queue(Queue queue);

/**
 * @brief Clears all elements from the queue
 *
 * Removes all elements from the queue and frees associated memory,
 * leaving the queue in an empty but usable state.
 *
 * @param[in,out] q Pointer to queue to be cleared
 *
 * @pre q must be a valid pointer to Queue structure
 * @post All queue elements are removed and memory is freed
 * @post Queue is reset to empty state (front and rear set to NULL)
 * @post Queue structure remains valid for further operations
 *
 * @note Equivalent to destroy_queue() but emphasizes reusability
 * @note Safe to call on empty queue
 * @warning Ensure no references to queue elements remain after clearing
 *
 * @see destroy_queue() for complete queue destruction
 * @see create_queue() for queue initialization
 *
 * @author Maulana Ishak
 */
void clear_queue(Queue* q);

#endif // QUEUE_H