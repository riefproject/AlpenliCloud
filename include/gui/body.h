#ifndef BODY_H
#define BODY_H

#include "raylib.h"

typedef struct Body {
    Rectangle panelRec;
    Rectangle panelContentRec;
    Rectangle panelView;
    Vector2 panelScroll;
    Rectangle currentZeroPosition;
    bool previousSelectedAll;  // Tambah di struct Body jika belum ada


    bool showCheckbox;
    bool selectedAll;
    int focusedIndex;
    bool selected[100]; // Untuk checkbox tiap baris
} Body;


void createBody(Body *body);

void updateBody(Body *body, Rectangle currentZeroPosition);

void drawBody(Body *body);

void drawTableItem(Body *body, int index, float startX, float startY, float rowHeight, float colWidths[4]);

void drawTableHeader(Body *body, float x, float y, float colWidths[]);

#endif
