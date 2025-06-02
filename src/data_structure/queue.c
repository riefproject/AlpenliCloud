// File: queue.c
// Author: Maulana Ishak
// Date: 09-04-2025
// Description: Implementation of queue functions in C
// License: MIT License

#include <stdio.h>
#include "queue.h"
#include "linked.h"

void create_queue(Queue* queue) {
  queue->front = NULL;
  queue->rear = NULL;
}

bool is_queue_empty(Queue queue) {
  if (queue.front == NULL && queue.rear == NULL) {
    return true;
  }
  return false;
}


void enqueue(Queue* queue, infotype data) {
  LinkedList temp_list;

  temp_list.head = queue->front;
  insert_last(&temp_list, data);

  queue->front = temp_list.head;
  if (queue->rear == NULL) {
    queue->rear = queue->front;
  }
  else {
    queue->rear = queue->rear->next;
  }

}

infotype dequeue(Queue* queue) {
  LinkedList temp_list;
  infotype data;
  temp_list.head = queue->front;
  delete_first(&temp_list, &data);
  queue->front = temp_list.head;
  if (queue->front == NULL) {
    queue->rear = NULL;
  }
  return data;
}

void print_queue(Queue queue) {
  LinkedList temp_list;
  temp_list.head = queue.front;
  if (is_queue_empty(queue)) {
    printf("[LOG] Queue is empty\n");
  }
  else {
    print_list(temp_list);
  }
}