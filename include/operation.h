#ifndef OPERATION_H
#define OPERATION_H

#include <stdio.h>
#include <stdlib.h>
typedef enum OperationType {
    CREATE,
    DELETE,
    UPDATE,
    RECOVER,
    SEARCH,
    UNDO,
    REDO,
    COPY,
    PASTE,
    CUT
} OperationType;

typedef struct Operation {
    char *from;
    char *to;
    OperationType type;
} Operation;

#endif // !OPERATION_H
