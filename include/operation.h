#ifndef OPERATION_H
#define OPERATION_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
typedef enum ActionType {
    ACTION_CREATE,
    ACTION_DELETE,
    ACTION_UPDATE,
    ACTION_RECOVER,
    ACTION_PASTE,
} ActionType ;

typedef struct Operation {
    char* from;
    char* to;
    ActionType type;
    bool isDir;
} Operation;

Operation createOperation(char* from, char* to, ActionType type);

#endif // !OPERATION_H
