#ifndef BODY_H
#define BODY_H

#include "raylib.h"

typedef struct Body
{
    Rectangle panelRec;         // adalah ukuran body
    Rectangle panelContentRec;  // adalah ukuran isi body
    Rectangle panelView;        // adalah ukuran yang terlihat di body (tinggi - tinggi scroll bawah dll)
    Vector2 panelScroll;        // adalah jauhnya scroll 

    Rectangle currentZeroPosition;
} Body;

void createBody(Body *body);

void updateBody(Body *body, Rectangle currentZeroPosition);

void drawBody(Body *body);

#endif
