// File: stack.c
// Author: Maulana Ishak
// Date: 09-04-2025
// Description: Implementation of stack functions in C
// License: MIT License

#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

// Initialize the stack
void create_stack(Stack* stack) {
    *stack = NULL;
}
// Check if the stack is empty
bool is_stack_empty(Stack stack) {
    return (stack == NULL);
}
// Push an element onto the stack
void push(Stack* stack, infotype data) {
    LinkedList temp_list;
    temp_list.head = *stack;
    insert_first(&temp_list, data);
    *stack = temp_list.head;
}

// Pop an element from the stack
void* pop(Stack* stack) {
    LinkedList temp_list;
    void* popped_data;
    temp_list.head = *stack;
    delete_first(&temp_list, &popped_data);
    *stack = temp_list.head;
    return popped_data;
}

void pop_print(Stack* stack, infotype* data) {
    LinkedList temp_list;
    temp_list.head = *stack;
    delete_first(&temp_list, data);
    *stack = temp_list.head;
    printf("[LOG] %d ", *data);
}

// Print the stack
void print_stack(Stack stack) {
    LinkedList temp_list;
    if (is_stack_empty(stack)) {
        printf("[LOG] Stack is empty\n");
        return;
    }
    temp_list.head = stack;
    print_list(temp_list);
}