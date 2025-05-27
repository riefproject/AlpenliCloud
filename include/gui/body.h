#ifndef BODY_H
#define BODY_H

#include "raylib.h"
#include "file_manager.h"

typedef struct Context Context;
typedef struct Body {
    Rectangle panelRec;
    Rectangle panelContentRec;
    Rectangle panelView;
    Vector2 panelScroll;
    Rectangle currentZeroPosition;

    bool showCheckbox;

    bool selectedAll;
    bool previousSelectedAll;

    int focusedIndex;
    bool selected[100];

    Context* ctx;
} Body;


void createBody(Context* ctx, Body* b);
void updateBody(Context* ctx, Body* body);
void drawBody(Context* ctx, Body* body);
void drawTableItem(Context* ctx, Body* body, Tree subTree, int index, float startX, float startY, float rowHeight, float colWidths[5]);
void drawTableHeader(Context* ctx, Body* body, float x, float y, float colWidths[]);
#endif
