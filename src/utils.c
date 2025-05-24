#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void inputString(char **s) {
    char *temp = malloc(1);
    char c;
    int i = 0;
    while ((c = getchar()) != '\n') {
        temp = realloc(temp, i + 1);
        temp[i] = c;

        if (temp == NULL) {
            printf("Memory allocation failed\n");
            break;
        }

        i++;
    }
    temp[i] = '\0';
    *s = temp;
}