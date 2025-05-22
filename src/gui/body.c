#include "gui/body.h"
#include "macro.h"
#include "item.h"
#include "file_manager.h"

#include "raygui.h"
#include <stdio.h>

void createBody(Body *b)
{
    Body body = {0};
    body.panelRec = (Rectangle){0};
    body.panelContentRec = (Rectangle){0, 0, 170, 340};
    body.panelView = (Rectangle){0};
    body.panelScroll = (Vector2){0};

    body.focusedIndex = -1;
    body.showCheckbox = true;
    body.selectedAll = false;
    body.previousSelectedAll = false;
    for (int i = 0; i < 100; i++)
    {
        body.selected[i] = false;
        // printf("-- %d:%d\n", i, body.selected[i]);
    }
    *b = body;
}

void updateBody(Body *body, Rectangle currentZeroPosition)
{
    body->currentZeroPosition = currentZeroPosition;

    body->panelRec = (Rectangle){
        body->currentZeroPosition.x + 170 + DEFAULT_PADDING,
        body->currentZeroPosition.y + DEFAULT_PADDING * 2 + 24 * 2,
        body->currentZeroPosition.width - 170 - DEFAULT_PADDING,
        body->currentZeroPosition.height - DEFAULT_PADDING * 2 - 24 * 2};

    if (body->selectedAll)
    {
        for (int i = 0; i < 100; i++)
        {
            body->selected[i] = body->selectedAll;
        }
    }
}

void drawBody(Body *body)
{
    float headerHeight = 30;
    float rowHeight = 24;

    float colWidths[5] = {300, 100, 100, 200};
    float checkboxWidth = body->showCheckbox ? 20 : 0;
    float totalContentWidth = checkboxWidth + colWidths[0] + colWidths[1] + colWidths[2] + colWidths[3];

    body->panelContentRec.width = totalContentWidth;

    GuiScrollPanel(body->panelRec, NULL, body->panelContentRec, &body->panelScroll, &body->panelView);

    BeginScissorMode(body->panelView.x, body->panelView.y, body->panelView.width, body->panelView.height);

    float startY = body->panelRec.y + headerHeight + body->panelScroll.y;
    float startX = body->panelRec.x + body->panelScroll.x;

    for (int i = 0; i < 100; i++)
    {
        drawTableItem(body, i, startX, body->panelRec.y + headerHeight + body->panelScroll.y, rowHeight, colWidths);

        if ((i + 1) * rowHeight + headerHeight > body->panelContentRec.height)
            body->panelContentRec.height = (i + 1) * rowHeight + headerHeight;
    }

    float headerX = body->panelRec.x + body->panelScroll.x;
    drawTableHeader(body, headerX, body->panelRec.y, colWidths);

    EndScissorMode();
}

void drawTableItem(Body *body, int index, float startX, float startY, float rowHeight, float colWidths[4])
{
    float checkboxWidth = body->showCheckbox ? 28 : 0;
    float totalContentWidth = checkboxWidth + colWidths[0] + colWidths[1] + colWidths[2] + colWidths[3];

    float rowY = startY + index * rowHeight;
    float rowX = startX;

    Rectangle rowRec = {rowX, rowY, totalContentWidth, rowHeight};

    if (CheckCollisionPointRec(GetMousePosition(), rowRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        body->focusedIndex = index;
    }

    Color bgColor = (body->focusedIndex == index) ? Fade(BLUE, 0.2f) : ((index % 2 == 0) ? WHITE : (Color){245, 245, 245, 255});
    DrawRectangleRec(rowRec, bgColor);

    float colX = rowX;

    if (body->showCheckbox)
    {
        Rectangle checkBox = {
            colX + 7,
            rowY + (rowHeight - 14) / 2,
            14, 14};

        GuiCheckBox(checkBox, NULL, &body->selected[index]);
        colX += checkboxWidth;
        if (index == 100)
        {
            exit(0);
        }
    }

    DrawText(TextFormat("File_%03d.txt", index), colX + 8, rowY + 6, 10, DARKGRAY);
    colX += colWidths[0];

    DrawText("Text File", colX + 8, rowY + 6, 10, DARKGRAY);
    colX += colWidths[1];

    DrawText("4 KB", colX + 8, rowY + 6, 10, DARKGRAY);
    colX += colWidths[2];

    DrawText("2025-05-21 10:00", colX + 8, rowY + 6, 10, DARKGRAY);
}

void drawTableHeader(Body *body, float x, float y, float colWidths[])
{
    int fontSize = 10;
    int headerHeight = 30;

    float colX = x;

    DrawRectangleRec((Rectangle){x, y, colWidths[0] + colWidths[1] + colWidths[2] + colWidths[3] + (body->showCheckbox ? 28 : 0), headerHeight}, LIGHTGRAY);
    DrawRectangleLinesEx((Rectangle){x, y, colWidths[0] + colWidths[1] + colWidths[2] + colWidths[3] + (body->showCheckbox ? 28 : 0), headerHeight}, 1, DARKGRAY);

    if (body->showCheckbox)
    {
        Rectangle checkRect = {
            colX + 7,
            y + (headerHeight - 14) / 2,
            14, 14};
        GuiCheckBox(checkRect, NULL, &body->selectedAll);
        colX += 28;
    }

    DrawRectangleLines(colX, y, colWidths[0], headerHeight, DARKGRAY);
    DrawText("Name", colX + 8, y + 8, fontSize, DARKGRAY);
    colX += colWidths[0];

    DrawRectangleLines(colX, y, colWidths[1], headerHeight, DARKGRAY);
    DrawText("Type", colX + 8, y + 8, fontSize, DARKGRAY);
    colX += colWidths[1];

    DrawRectangleLines(colX, y, colWidths[2], headerHeight, DARKGRAY);
    DrawText("Size", colX + 8, y + 8, fontSize, DARKGRAY);
    colX += colWidths[2];

    DrawRectangleLines(colX, y, colWidths[3], headerHeight, DARKGRAY);
    DrawText("Modified Time", colX + 8, y + 8, fontSize, DARKGRAY);
}
