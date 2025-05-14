#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "file_manager.h"
int main() {
    // printf("Hello World!\n");
    // char* cmd = "mkdir";

    if(system("mkdir aku_disini") == 0){
        printf("folder berhasil dibuat\n");
    }else{
        printf("Jancok");
    };
    // system(cmd);

    return 0;
}