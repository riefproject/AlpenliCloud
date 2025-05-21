// File: linked_list.c
// Author: Maulana Ishak
// Date: 06-04-2025
// Description: Implementation of linked list functions in C
// License: MIT License

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "linked.h"

// Initialize the linked list
void create_list(LinkedList* list){
  list->head = NULL;
}

// Create a new node with the given data
Node* create_node(void* data){
  Node* new_node = (Node*)malloc(sizeof(Node));
  if(new_node == NULL){
    printf("Overflow, Memory allocation failed\n");
    return NULL;
  }
  new_node->data = data;
  new_node->next = NULL;
  return new_node;
}

// Check if the list is empty
bool is_list_empty(LinkedList list){
  if(list.head == NULL){
    return true;
  }
  return false;
}

// Check if given data exists in the list
bool is_exist(LinkedList list, infotype data){
  Node* curr;

  curr = list.head;
  while(curr != NULL){
    if(curr->data == data){
      return true;
    }else{
      curr = curr->next;
    }
  }

  return false;
}


// Print the linked list in forward order
void print_list(LinkedList list){
  Node* curr;
  if(is_list_empty(list)){
    printf("list empty\n");
  }else{
    curr = list.head;
    while(curr->next != NULL){
      printf("[%c]->", curr->data);
      curr = curr->next;
    }
    printf("[%c]\n", curr->data);
  }
}

void destroy_list(LinkedList* list){
  Node* curr;
  Node* temp;

  curr = list->head;
  while(curr != NULL){
    temp = curr;
    curr = curr->next;

    temp->next = NULL;
    free(temp);
  }
  list->head = NULL;
}

// Get the length of the linked list
int get_length(LinkedList list){
  Node* curr;
  int count;

  curr = list.head;
  count = 0;

  while(curr != NULL){
    count++;
    curr = curr->next;
  }

  return count;
  
}


void insert_first(LinkedList* list, infotype data){
  Node* new_node;
  new_node = create_node(data);
  if(new_node == NULL) return;

  if(is_list_empty(*list)){
    list->head = new_node;
  }else{
    new_node->next = list->head;
    list->head = new_node;
  }
}

void insert_last(LinkedList* list, infotype data){
  Node  *new_node, *curr;
  new_node = create_node(data);
  if(new_node == NULL) return;

  curr = list->head;
  if(curr == NULL){
    insert_first(list, data);
  }else{
    while(curr->next != NULL){
      curr = curr->next;
    }
    curr->next = new_node;
  }
}

void insert_at(LinkedList* list, int position, infotype data){
  Node *new_node, *curr;
  new_node = create_node(data);
  if(new_node == NULL) return;

  if(position < 1 || position > get_length(*list)){
    printf("Invalid position\n");
    return;
  }

  if(position == 1){
    insert_first(list, data);
  }else{
    curr = list->head;
    for(int i = 1; i < position-1; i++){
      curr = curr->next;
    }
    new_node->next = curr->next;
    curr->next = new_node;
  }
}

void delete_first(LinkedList* list, infotype* temp){
  Node* curr;
  if(is_list_empty(*list)){
    printf("List is empty\n");
    return;
  }else{
    curr = list->head;
    list->head = curr->next;
    curr->next = NULL;

    *temp = curr->data;
    free(curr);
    curr = NULL;
  }
}
void delete_last(LinkedList* list, infotype* temp){
  Node *curr, *prev;
  if(is_list_empty(*list)){
    printf("List is empty\n");
    return;
  }else{
    curr = list->head;
    prev = NULL;
    while(curr->next != NULL){
      prev = curr;
      curr = curr->next;
    }
    *temp = curr->data;
    if(prev == NULL){
      free(curr);
      list->head = NULL;
    }else{
      free(curr);
      prev->next = NULL;
    }
  }
}



void delete_val(LinkedList* list, infotype data, infotype* temp){
  Node *curr, *prev;
  if(is_list_empty(*list)){
    printf("List is empty\n");
    return;
  }else{
    curr = list->head;
    prev = NULL;
    while(curr != NULL && curr->data != data){
      prev = curr;
      curr = curr->next;
    }
    if(curr == NULL){
      printf("Node with data %d not found\n", data);
      return;
    }else{
      if(prev == NULL){
        list->head = curr->next;
      }else{
        prev->next = curr->next;
      }
      *temp = curr->data;
      free(curr);
    }
  }
}

void delete_at(LinkedList* list, int position, infotype* temp){
  Node *curr, *prev;
  if(is_list_empty(*list)){
    printf("List is empty\n");
    return;
  }else{
    curr = list->head;
    prev = NULL;
    if(position < 1 || position > get_length(*list)){
      printf("Invalid position\n");
      return;
    }
    for(int i = 1; i < position; i++){
      prev = curr;
      curr = curr->next;
    }
    if(prev == NULL){
      list->head = curr->next;
    }else{
      prev->next = curr->next;
    }
    *temp = curr->data;
    free(curr);
  }
}
