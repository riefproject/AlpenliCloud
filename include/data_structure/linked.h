// File: linked_list.h
// Author: Maulana Ishak
// Date: 06-04-2025
// Description: Header file for linked list implementation in C
// License: MIT License

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdbool.h>

#define infotype void*

// Define the structure for a node in the linked list
typedef struct Node {
    infotype data;
    struct Node* next;
} Node;

typedef struct LinkedList {
    Node* head;
} LinkedList;

// Utility Functions
// Initialize the linked list
void create_list(LinkedList* list);
// Create a new node with the given data
Node* create_node(infotype data);
// Check if the list is empty
bool is_list_empty(LinkedList list);
// Check if given data exists in the list
bool is_exist(LinkedList list, infotype data);
// Get the length of the linked list
int get_length(LinkedList list);

// destroy the linked list and free memory
void destroy_list(LinkedList* list);
// Print the linked list in forward order
void print_list(LinkedList list);


// Insert Operations
// Insert a new node at the beginning of the list
void insert_first(LinkedList* list, infotype data);
// Insert a new node at the end of the list
void insert_last(LinkedList* list, infotype data);
// Insert a new node at the given position in the list
void insert_at(LinkedList* list, int position, infotype data);

// Delete Operations
// Delete the first node in the list
void delete_first(LinkedList* list, infotype* temp);
// Delete the last node in the list
void delete_last(LinkedList* list, infotype* temp);
// Delete the node with the given data in the list
void delete_val(LinkedList* list, infotype data, infotype* temp);
// Delete the node at the given position in the list
void delete_at(LinkedList* list, int position, infotype* temp);



#endif // !LINKED_LIST_H