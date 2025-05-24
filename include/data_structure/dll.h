#ifndef DLL_H
#define DLL_H

#ifndef ALOCATE_SHORTCUTS
#define ALOCATE_SHORTCUTS
#define new(type) (type*)malloc(sizeof(type))
#define delete(ptr) do { free(ptr); ptr = NULL; } while (0)
#endif

#include <stdbool.h>

typedef struct SingleLinkedList;
typedef struct DLLNode;

typedef struct DLLNode {
    void* data;
    DLLNode* next;
    DLLNode* prev;
}DLLNode;

typedef struct DoublyLinkedList {
    DLLNode* head;
    DLLNode* tail;
    int size;
}DoublyLinkedList;


// ===================================================
//            . . . DOUBLY LINKED LIST . . .
// ===================================================

/*      CONSTRUCTOR
 * ==================== */
DoublyLinkedList* createDoublyLinkedList();
DLLNode* createDLLNode(void* data);

/*      GETTERS
 * ==================== */
void* DLL_getNodeData(DLLNode* node);
DLLNode* DLL_getNextNode(DLLNode* node);
DLLNode* DLL_getPrevNode(DLLNode* node);

/*      SETTERS
 * ==================== */
void DLL_setNodeData(DLLNode* node, void* data);
void DLL_setNextNode(DLLNode* node, DLLNode* next);
void DLL_setPrevNode(DLLNode* node, DLLNode* prev);

/*     INSERT NODE
 * ==================== */
void DLL_insertFront(DoublyLinkedList* list, void* data);
void DLL_insertBack(DoublyLinkedList* list, void* data);
void DLL_insertNode(DoublyLinkedList* list, void* data, int pos);

/*     REMOVE NODE
 * ==================== */
void DLL_removeFront(DoublyLinkedList* list, DLLNode* node);
void DLL_removeBack(DoublyLinkedList* list, DLLNode* node);
void DLL_removeNode(DoublyLinkedList* list, DLLNode* node, int pos);

/*       GET NODE
 * ==================== */
void* DLL_getFront(DoublyLinkedList* list);
void* DLL_getBack(DoublyLinkedList* list);
void* DLL_getNode(DoublyLinkedList* list, int pos);

/*  DEALOC AND DESTRUCT
 * ==================== */
void DLL_clearList(DoublyLinkedList* list);
void DLL_freeList(DoublyLinkedList* list);
void DLL_freeNode(DLLNode* node);

/*      PRINT LIST
 * ==================== */
void DLL_printList(DoublyLinkedList* list, void (*printFunc)(void*));
void DLL_printListReverse(DoublyLinkedList* list, void (*printFunc)(void*));

#endif