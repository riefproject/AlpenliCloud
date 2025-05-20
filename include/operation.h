#ifndef OPERATION_H
#define OPERATION_H

#include <stdio.h>
#include <stdlib.h>
typedef enum OperationType {
    TYPE_CREATE,
    TYPE_DELETE,
    TYPE_UPDATE,
    TYPE_RECOVER,
    TYPE_PASTE,
    TYPE_PATH
} OperationType;

typedef struct Operation {
    char *from;
    char *to;
    OperationType type;
} Operation;

Operation createOperation(char* from, char* to, OperationType type);

#endif // !OPERATION_H
