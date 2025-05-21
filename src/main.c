#define RAYGUI_IMPLEMENTATION

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include "raygui.h"
#include "raylib.h"
#include "gui/titlebar.h"
#include "gui/component.h"
#include "gui/navbar.h"

int main()
{
    // Windows config
    // ----------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;
    int padding = 10;
    
    TitleBar titleBar;
    createTitleBar(&titleBar, screenWidth, screenHeight);
    
    Rectangle currentZeroPosition = {padding, titleBar.height + padding, screenWidth - padding * 2, screenHeight - titleBar.height - padding * 2}; 
    
    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    SetTargetFPS(60);
    
    InitWindow(screenWidth, screenHeight, "raygui - controls test suite");
    // ----------------------------------------------------------------------------------------
    
    // initialization
    // ----------------------------------------------------------------------------------------

    Navbar navbar;
    createNavbar(&navbar);

    Rectangle panelRec = {currentZeroPosition.x, currentZeroPosition.y + 100, 200, currentZeroPosition.height - 100};
    Rectangle panelContentRec = {0, 0, 340, 340};
    Rectangle panelView = {0};
    Vector2 panelScroll = {0};

    while (!titleBar.exitWindow && !WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        updateTitleBar(&titleBar);

        updateNavbar(&navbar, currentZeroPosition);
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        drawTitleBar(&titleBar);

        drawNavbar(&navbar);

        GuiScrollPanel(panelRec, NULL, panelContentRec, &panelScroll, &panelView);

        BeginScissorMode(panelView.x, panelView.y, panelView.width, panelView.height);
            for (size_t i = 0; i < 1000; i++)
            {
                // Rectangle rec = {panelScroll.x + (i % 10) * 50, panelScroll.y + (i / 10) * 50, 40, 40};
                // DrawRectangleRec(rec, (Color){(unsigned char)(GetRandomValue(0, 255)), (unsigned char)(GetRandomValue(0, 255)), (unsigned char)(GetRandomValue(0, 255)), 255});
                // DrawRectangleLinesEx(rec, 1.0f, BLACK);
                panelContentRec.height = i * 40 > panelContentRec.height ? i * 40 : panelContentRec.height; 
                DrawText(TextFormat("Mouse Position: [ %.0f, %.0f ]", panelView.width, panelView.height), panelScroll.x, panelScroll.y + (i * 40), 10, DARKGRAY);
            }
            
            // DrawText(TextFormat("Window Position: [ %.0f, %.0f ]", panelRec.x + panelScroll.x, panelRec.y + panelScroll.y), panelRec.x + panelScroll.x, panelRec.y + panelScroll.y, 10, DARKGRAY);
            GuiGrid((Rectangle){panelRec.x + panelScroll.x, panelRec.y + panelScroll.y, panelContentRec.width, panelContentRec.height}, NULL, 16, 3, NULL);
        EndScissorMode();

        // GuiCheckBox((Rectangle){565, 80, 20, 20}, "SHOW CONTENT AREA", &showContentArea);

        // GuiSliderBar((Rectangle){590, 385, 145, 15}, "WIDTH", TextFormat("%i", (int)panelContentRec.width), &panelContentRec.width, 1, 600);
        // GuiSliderBar((Rectangle){590, 410, 145, 15}, "HEIGHT", TextFormat("%i", (int)panelContentRec.height), &panelContentRec.height, 1, 1000);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
