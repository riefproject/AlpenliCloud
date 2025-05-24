#include "operation.h"
#include <string.h>

Operation createOperation(char *from, char *to, OperationType type) {
    return (Operation){
        .from = strdup(from),
        .to = strdup(to),
        .type = type};
}