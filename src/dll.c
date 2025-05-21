#include "dll.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>

// ===================================================
//            . . . DOUBLY LINKED LIST . . .
// ===================================================

/*      CONSTRUCTOR
 * ==================== */

DoublyLinkedList* createDoublyLinkedList() {
    DoublyLinkedList* list = new(DoublyLinkedList);
    if (list == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for DoublyLinkedList\n");
        return NULL;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

DLLNode* createDLLNode(void* data) {
    DLLNode* node = new(DLLNode);
    if (node == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for DLLNode\n");
        return NULL;
    }
    node->data = data;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

/*      GETTERS
 * ==================== */

void* DLL_getNodeData(DLLNode* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: DLLNode is NULL\n");
        return NULL;
    }
    return node->data;
}

DLLNode* DLL_getNextNode(DLLNode* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: DLLNode is NULL\n");
        return NULL;
    }
    return node->next;
}

DLLNode* DLL_getPrevNode(DLLNode* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: DLLNode is NULL\n");
        return NULL;
    }
    return node->prev;
}

/*      SETTERS
 * ==================== */

void DLL_setNodeData(DLLNode* node, void* data) {
    if (node == NULL) {
        fprintf(stderr, "Error: DLLNode is NULL\n");
        return;
    }
    node->data = data;
}

void DLL_setNextNode(DLLNode* node, DLLNode* next) {
    if (node == NULL) {
        fprintf(stderr, "Error: DLLNode is NULL\n");
        return;
    }
    node->next = next;
}

void DLL_setPrevNode(DLLNode* node, DLLNode* prev) {
    if (node == NULL) {
        fprintf(stderr, "Error: DLLNode is NULL\n");
        return;
    }
    node->prev = prev;
}

/*     INSERT NODE
 * ==================== */

void DLL_insertFront(DoublyLinkedList* list, void* data) {
    DLLNode* newNode = createDLLNode(data);
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for new node\n");
        return;
    }
    if (list->head == NULL) {
        list->head = newNode;
        list->tail = newNode;
    }
    else {
        newNode->next = list->head;
        list->head->prev = newNode;
        list->head = newNode;
    }
    list->size++;
}

void DLL_insertBack(DoublyLinkedList* list, void* data) {
    DLLNode* newNode = createDLLNode(data);
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for new node\n");
        return;
    }
    if (list->tail == NULL) {
        list->head = newNode;
        list->tail = newNode;
    }
    else {
        newNode->prev = list->tail;
        list->tail->next = newNode;
        list->tail = newNode;
    }
    list->size++;
}

void DLL_insertNode(DoublyLinkedList* list, void* data, int pos) {
    if (pos < 0 || pos > list->size) {
        fprintf(stderr, "Error: Invalid position for insertion\n");
        return;
    }
    if (pos == 0) {
        DLL_insertFront(list, data);
        return;
    }
    if (pos == list->size) {
        DLL_insertBack(list, data);
        return;
    }
    DLLNode* newNode = createDLLNode(data);
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for new node\n");
        return;
    }
    DLLNode* current = list->head;
    for (int i = 0; i < pos - 1; i++) {
        current = current->next;
    }
    newNode->next = current->next;
    newNode->prev = current;
    if (current->next != NULL) {
        current->next->prev = newNode;
    }
    current->next = newNode;
    list->size++;
}

/*     REMOVE NODE
 * ==================== */

void DLL_removeFront(DoublyLinkedList* list, DLLNode* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: DLLNode is NULL\n");
        return;
    }
    if (list->head == node) {
        list->head = node->next;
        if (list->head != NULL) {
            list->head->prev = NULL;
        }
        else {
            list->tail = NULL;
        }
    }
    else {
        node->prev->next = node->next;
        if (node->next != NULL) {
            node->next->prev = node->prev;
        }
    }
    DLL_freeNode(node);
    list->size--;
}

void DLL_removeBack(DoublyLinkedList* list, DLLNode* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: DLLNode is NULL\n");
        return;
    }
    if (list->tail == node) {
        list->tail = node->prev;
        if (list->tail != NULL) {
            list->tail->next = NULL;
        }
        else {
            list->head = NULL;
        }
    }
    else {
        node->next->prev = node->prev;
        if (node->prev != NULL) {
            node->prev->next = node->next;
        }
    }
    DLL_freeNode(node);
    list->size--;
}

void DLL_removeNode(DoublyLinkedList* list, DLLNode* node, int pos) {
    if (node == NULL) {
        fprintf(stderr, "Error: DLLNode is NULL\n");
        return;
    }
    if (pos < 0 || pos >= list->size) {
        fprintf(stderr, "Error: Invalid position for removal\n");
        return;
    }
    if (pos == 0) {
        DLL_removeFront(list, node);
        return;
    }
    if (pos == list->size - 1) {
        DLL_removeBack(list, node);
        return;
    }
    if (node->prev != NULL) {
        node->prev->next = node->next;
    }
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }
    DLL_freeNode(node);
    list->size--;
}

/*       GET NODE
 * ==================== */

void* DLL_getFront(DoublyLinkedList* list) {
    if (list->head == NULL) {
        fprintf(stderr, "Error: List is empty\n");
        return NULL;
    }
    return list->head->data;
}

void* DLL_getBack(DoublyLinkedList* list) {
    if (list->tail == NULL) {
        fprintf(stderr, "Error: List is empty\n");
        return NULL;
    }
    return list->tail->data;
}

void* DLL_getNode(DoublyLinkedList* list, int pos) {
    if (pos < 0 || pos >= list->size) {
        fprintf(stderr, "Error: Invalid position for retrieval\n");
        return NULL;
    }
    DLLNode* current = list->head;
    for (int i = 0; i < pos; i++) {
        current = current->next;
    }
    return current->data;
}

/*  DEALOC AND DESTRUCT
 * ==================== */

void DLL_clearList(DoublyLinkedList* list) {
    DLLNode* current = list->head;
    while (current != NULL) {
        DLLNode* next = current->next;
        DLL_freeNode(current);
        current = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void DLL_freeList(DoublyLinkedList* list) {
    if (list == NULL) {
        fprintf(stderr, "Error: List is NULL\n");
        return;
    }
    DLL_clearList(list);
    delete(list);
    list = NULL;
}

void DLL_freeNode(DLLNode* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: DLLNode is NULL\n");
        return;
    }
    delete(node->data);
    delete(node);
    node = NULL;
}

/*      PRINT LIST
 * ==================== */

void DLL_printList(DoublyLinkedList* list, void (*printFunc)(void*)) {
    if (list == NULL) {
        fprintf(stderr, "Error: List is NULL\n");
        return;
    }
    DLLNode* current = list->head;
    while (current != NULL) {
        printFunc(current->data);
        current = current->next;
    }
    printf("\n");
}

void DLL_printListReverse(DoublyLinkedList* list, void (*printFunc)(void*)) {
    if (list == NULL) {
        fprintf(stderr, "Error: List is NULL\n");
        return;
    }
    DLLNode* current = list->tail;
    while (current != NULL) {
        printFunc(current->data);
        current = current->prev;
    }
    printf("\n");
}
