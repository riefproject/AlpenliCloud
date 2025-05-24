#pragma once
#include <stdbool.h>
typedef enum ActType {
    ACTION_MOVE,
    ACTION_DELETE,
    ACTION_RENAME,
    ACTION_CREATE
} ActType;

typedef struct Action {
    ActType type;
    char* src;
    char* dst;
    bool isDir;
}Action;