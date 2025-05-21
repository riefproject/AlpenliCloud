#include "gui/body.h"
#include "gui/component.h"

#include "raygui.h"
#include <stdio.h>
void createBody(Body *body)
{
    body->panelRec = (Rectangle) {0};
    body->panelContentRec = (Rectangle){0, 0, 170, 340};
    body->panelView = (Rectangle){0};
    body->panelScroll = (Vector2){0};
}

void updateBody(Body *body, Rectangle currentZeroPosition)
{
    body->currentZeroPosition = currentZeroPosition;
    body->panelRec = (Rectangle) {
        body->currentZeroPosition.x + 170 + DEFAULT_PADDING, 
        body->currentZeroPosition.y + DEFAULT_PADDING + 24, 
        body->currentZeroPosition.width - 170 - DEFAULT_PADDING, 
        body->currentZeroPosition.height - DEFAULT_PADDING - 24
    };
}

void drawBody(Body *body)
{
    GuiScrollPanel(body->panelRec, NULL, body->panelContentRec, &body->panelScroll, &body->panelView);

    BeginScissorMode(body->panelView.x, body->panelView.y, body->panelView.width, body->panelView.height);
    for (int i = 0; i < 1000; i++)
    {

        body->panelContentRec.height = i * 40 > body->panelContentRec.height ? i * 40 : body->panelContentRec.height;
        DrawText(TextFormat("Mouse Position: [ %.0f, %.0f ]", body->panelView.width, body->panelView.height), body->panelRec.x + body->panelScroll.x, body->panelRec.y + body->panelScroll.y + (i * 40), 10, DARKGRAY);
    }

    GuiGrid((Rectangle){body->panelRec.x + body->panelScroll.x, body->panelRec.y + body->panelScroll.y, body->panelContentRec.width, body->panelContentRec.height}, NULL, 16, 3, NULL);
    EndScissorMode();
}