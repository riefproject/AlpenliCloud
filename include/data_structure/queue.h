// File: queue.h
// Author: Maulana Ishak
// Date: 09-04-2025
// Description: Header file for queue implementation in C
// License: MIT License

#ifndef QUEUE_H
#define QUEUE_H

#include "linked.h"
#include "item.h"

typedef struct Queue {
  Node* front;
  Node* rear;
} Queue;

// Function to create a queue
void create_queue(Queue* queue);
// Function to check if the queue is empty
bool is_queue_empty(Queue queue);
// Function to enqueue an element into the queue
void enqueue(Queue* queue, infotype data);
// Function to dequeue an element from the queue
infotype dequeue(Queue* queue);
// Function to get the front element of the queue
void destroy_queue(Queue* queue);
// Function to print the queue
void print_queue(Queue queue);
void clear_queue(Queue* q);

#endif // QUEUE_H