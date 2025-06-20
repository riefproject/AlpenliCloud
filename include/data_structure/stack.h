#ifndef STACK_H
#define STACK_H

#include "linked.h"

/**
 * @file stack.h
 * @brief Stack data structure implementation using linked list
 * @author Maulana Ishak
 * @date 2025-04-09
 * @license MIT License
 */

 /*
 ====================================================================
     TYPE DEFINITIONS
 ====================================================================
 */

 /**
  * @brief Stack data structure typedef
  *
  * Stack implementation using linked list nodes. The stack follows
  * Last-In-First-Out (LIFO) principle for data storage and retrieval.
  */
typedef Node* Stack;

/*
====================================================================
    STACK OPERATIONS
====================================================================
*/

/**
 * @brief Creates and initializes an empty stack
 *
 * Initializes the stack pointer to NULL, creating an empty stack
 * ready for push and pop operations.
 *
 * @param[out] stack Pointer to stack to be initialized
 *
 * @pre stack must be a valid pointer to Stack
 * @post Stack is initialized to NULL (empty state)
 *
 * @note Stack is implemented using linked list with NULL indicating empty stack
 * @warning Ensure stack pointer is valid before calling this function
 *
 * @see push() for adding elements to stack
 * @see pop() for removing elements from stack
 *
 * @author Maulana Ishak
 */
void create_stack(Stack* stack);

/**
 * @brief Checks if the stack is empty
 *
 * Determines whether the stack contains any elements by checking
 * if the stack pointer is NULL.
 *
 * @param[in] stack Stack to check for emptiness
 *
 * @return bool Stack emptiness status
 * @retval true Stack is empty (no elements)
 * @retval false Stack contains one or more elements
 *
 * @pre stack can be NULL or valid Stack
 * @post No modification to stack state
 *
 * @note Safe to call with NULL stack (returns true)
 *
 * @see create_stack() for stack initialization
 *
 * @author Maulana Ishak
 */
bool is_stack_empty(Stack stack);

/**
 * @brief Pushes an element onto the top of the stack
 *
 * Adds a new element to the top of the stack using linked list insertion.
 * The new element becomes the new top of the stack.
 *
 * @param[in,out] stack Pointer to stack to push element onto
 * @param[in] data Data to be pushed onto stack
 *
 * @pre stack must be a valid pointer to Stack
 * @pre data must be a valid infotype value
 * @post New element is added to top of stack
 * @post Stack size increases by one
 * @post Previous top element (if any) becomes second element
 *
 * @note Memory allocation is performed for new stack element
 * @note Stack can grow indefinitely (limited only by available memory)
 * @warning Ensure sufficient memory is available for stack growth
 *
 * @see pop() for removing elements from stack
 * @see is_stack_empty() for checking stack state
 *
 * @author Maulana Ishak
 */
void push(Stack* stack, infotype data);

/**
 * @brief Pops and returns the top element from the stack
 *
 * Removes and returns the top element from the stack. The element
 * is removed from the stack and its data is returned to caller.
 *
 * @param[in,out] stack Pointer to stack to pop element from
 *
 * @return void* Pointer to data from popped element
 * @retval void* Valid data pointer if stack is not empty
 * @retval NULL If stack is empty or error occurs
 *
 * @pre stack must be a valid pointer to Stack
 * @post Top element is removed from stack if not empty
 * @post Stack size decreases by one if not empty
 * @post Memory for popped node is freed
 *
 * @note Returns NULL for empty stack
 * @note Caller is responsible for proper casting of returned data
 * @warning Check return value for NULL before using returned data
 *
 * @see push() for adding elements to stack
 * @see pop_print() for pop operation with data output
 *
 * @author Maulana Ishak
 */
void* pop(Stack* stack);

/**
 * @brief Pops element from stack and stores data in provided variable
 *
 * Removes the top element from stack and copies its data to the
 * provided data parameter. Provides safe pop operation with data retrieval.
 *
 * @param[in,out] stack Pointer to stack to pop element from
 * @param[out] data Pointer to variable to store popped data
 *
 * @pre stack must be a valid pointer to Stack
 * @pre data must be a valid pointer to infotype variable
 * @post Top element is removed from stack if not empty
 * @post Data from popped element is copied to data parameter
 * @post Stack size decreases by one if not empty
 * @post Memory for popped node is freed
 *
 * @note No operation performed if stack is empty
 * @note Safer alternative to pop() for data retrieval
 *
 * @see pop() for pop operation with return value
 * @see push() for adding elements to stack
 *
 * @author Maulana Ishak
 */
void pop_print(Stack* stack, infotype* data);

/**
 * @brief Prints all elements in the stack from top to bottom
 *
 * Displays all elements in the stack in order from top to bottom
 * without modifying the stack structure. Useful for debugging and visualization.
 *
 * @param[in] stack Stack to print elements from
 *
 * @pre stack can be NULL or valid Stack
 * @post No modification to stack state
 * @post Elements are printed to standard output from top to bottom
 *
 * @note Safe to call with empty stack (prints empty stack message)
 * @note Does not modify stack contents
 * @note Output format depends on infotype data structure
 *
 * @see is_stack_empty() for checking stack state
 *
 * @author Maulana Ishak
 */
void print_stack(Stack stack);

#endif // STACK_H