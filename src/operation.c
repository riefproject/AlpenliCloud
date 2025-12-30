#include "operation.h"
#include <string.h>

Operation createOperation(char* from, char* to, ActionType type, bool isDir, Queue* itemTemp) {
    return (Operation) {
        .from = from ? strdup(from) : NULL,
        .to = to ? strdup(to) : NULL,
        .type = type,
        .isDir = isDir,
        .itemTemp = itemTemp
    };
}
