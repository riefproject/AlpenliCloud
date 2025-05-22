#ifndef BODY_H
#define BODY_H

#include "raylib.h"
#include "file_manager.h"

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

    FileManager *fileManager;
} Body;


void createBody(Body *body);

void updateBody(Body *body, Rectangle currentZeroPosition, FileManager *fileManager);

void drawBody(Body *body);

void drawTableItem(Body *body, Item item, int index, float startX, float startY, float rowHeight, float colWidths[5]);

void drawTableHeader(Body *body, float x, float y, float colWidths[]);

#endif
