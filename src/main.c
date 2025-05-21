#define RAYGUI_IMPLEMENTATION

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include "raygui.h"
#include "raylib.h"
#include "gui/component.h"
#include "gui/titlebar.h"
#include "gui/navbar.h"
#include "gui/sidebar.h"
#include "gui/body.h"

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
    
    Sidebar sidebar;
    createSidebar(&sidebar);

    Body body;
    createBody(&body);
    


    while (!titleBar.exitWindow && !WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        updateTitleBar(&titleBar);

        updateNavbar(&navbar, currentZeroPosition);

        updateSidebar(&sidebar, currentZeroPosition);

        updateBody(&body, currentZeroPosition);

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        drawTitleBar(&titleBar);

        drawBody(&body);
        
        drawSidebar(&sidebar);
        
        drawNavbar(&navbar);
        
        

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
