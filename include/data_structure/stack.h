// File: stack.h
// Author: Maulana Ishak
// Date: 09-04-2025
// Description: Header file for stack implementation in C
// License: MIT License

#ifndef STACK_H
#define STACK_H

#include "linked.h"

typedef Node* Stack;

void create_stack(Stack* stack);
bool is_stack_empty(Stack stack);
void push(Stack* stack, infotype data);
void* pop(Stack* stack);
void pop_print(Stack* stack, infotype* data);
void print_stack(Stack stack);

#endif // STACK_H