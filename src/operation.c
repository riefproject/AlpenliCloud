#include "operation.h"
#include <string.h>s

Operation createOperation(char* from, char* to, OperationType type){
  return (Operation){
    .from = strdup(from),
    .to = strdup(to),
    .type = type
  };
}