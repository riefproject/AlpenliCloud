#include "operation.h"
#include <string.h>

Operation createOperation(char *from, char *to, ActionType type, bool isDir, Queue *itemTemp) {
    return (Operation){
        .from = strdup(from),
        .to = strdup(to),
        .type = type,
        .isDir = isDir,
        .itemTemp = itemTemp
    };
}