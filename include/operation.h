#ifndef OPERATION_H
#define OPERATION_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "queue.h"

/*
====================================================================
    ENUMERASI DAN STRUKTUR DATA
====================================================================
*/

typedef enum ActionType {
    ACTION_CREATE,
    ACTION_DELETE,
    ACTION_UPDATE,
    ACTION_RECOVER,
    ACTION_PASTE,
} ActionType;

typedef struct Operation {
    char* from;        // path asal operasi
    char* to;          // path tujuan operasi
    ActionType type;   // tipe operasi yang dilakukan
    bool isDir;        // flag apakah operasi pada direktori
    Queue* itemTemp;   // queue temporary untuk menyimpan item
} Operation;

/*
====================================================================
    KONSTRUKTOR OPERATION
====================================================================
*/

// Function create operation
// Membuat struct Operation dengan field from dan to menggunakan strdup dan itemTemp diset NULL
// IS: Parameter from, to, dan type diketahui dan valid
// FS: Operation baru dibuat dengan from dan to di-copy menggunakan strdup, type diset sesuai parameter, isDir dan itemTemp diinisialisasi
// Created by: 
Operation createOperation(char* from, char* to, ActionType type);

#endif // !OPERATION_H