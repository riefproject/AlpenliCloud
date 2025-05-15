#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "linked.h"

// Initialize the stack
void create_stack(Stack* stack) {
    *stack = NULL;
}
// Check if the stack is empty
bool is_stack_empty(Stack stack) {
    return (stack == NULL);
}
// Push an element onto the stack
void push(Stack* stack, infotype data){
  List temp_list;
  temp_list.head = *stack;
  insert_first(&temp_list, data);
  *stack = temp_list.head;
}

// Pop an element from the stack
void pop(Stack* stack, infotype* data){
  List temp_list;
  temp_list.head = *stack;
  delete_first(&temp_list, data);
  *stack = temp_list.head;
}

void pop_print(Stack* stack, infotype* data){
    List temp_list;
    temp_list.head = *stack;
    delete_first(&temp_list, data);
    *stack = temp_list.head;
    printf("%d ", *data);
}

// Print the stack
void print_stack(Stack stack) {
    List temp_list;
    if(is_stack_empty(stack)) {
        printf("Stack is empty\n");
        return;
    }
    temp_list.head = stack;
    print_list(temp_list);
}