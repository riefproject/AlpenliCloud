#include <stdio.h>

#include "gui/sidebar.h"
#include "raygui.h"
#include "macro.h"

void createSidebar(Sidebar *sidebar)
{
    sidebar->panelRec = (Rectangle) {0};
    sidebar->panelContentRec = (Rectangle){0, 0, 170, 340};
    sidebar->panelView = (Rectangle){0};
    sidebar->panelScroll = (Vector2){0};
}

void updateSidebar(Sidebar *sidebar, Rectangle currentZeroPosition)
{
    sidebar->currentZeroPosition = currentZeroPosition;
    sidebar->currentZeroPosition.y =+ DEFAULT_PADDING * 3 + 24 * 3;
    sidebar->currentZeroPosition.height =+ sidebar->currentZeroPosition.height - DEFAULT_PADDING * 2 - 24 * 2;

    sidebar->panelRec = (Rectangle){sidebar->currentZeroPosition.x, sidebar->currentZeroPosition.y, 170, sidebar->currentZeroPosition.height};
}

void drawSidebar(Sidebar *sidebar)
{
    GuiScrollPanel(sidebar->panelRec, NULL, sidebar->panelContentRec, &sidebar->panelScroll, &sidebar->panelView);

    BeginScissorMode(sidebar->panelView.x, sidebar->panelView.y, sidebar->panelView.width, sidebar->panelView.height);
    for (int i = 0; i < 1000; i++)
    {
        sidebar->panelContentRec.height = i * 40 > sidebar->panelContentRec.height ? i * 40 : sidebar->panelContentRec.height;
        DrawText(TextFormat("Mouse Position: [ %.0f, %.0f ]", sidebar->panelView.width, sidebar->panelView.height), sidebar->panelRec.x + sidebar->panelScroll.x, sidebar->panelRec.y + sidebar->panelScroll.y + (i * 40), 10, DARKGRAY);
    }

    GuiGrid((Rectangle){sidebar->panelRec.x + sidebar->panelScroll.x, sidebar->panelRec.y + sidebar->panelScroll.y, sidebar->panelContentRec.width, sidebar->panelContentRec.height}, NULL, 16, 3, NULL);
    EndScissorMode();
}