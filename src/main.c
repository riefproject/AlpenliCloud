#define RAYGUI_IMPLEMENTATION

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include "raygui.h"
#include "raylib.h"
#include "macro.h"

#include "gui/titlebar.h"
#include "gui/navbar.h"
#include "gui/toolbar.h"
#include "gui/sidebar.h"
#include "gui/body.h"
#include "file_manager.h"

int main()
{
    // Windows config
    // ----------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;
    
    FileManager fileManager;
    
    createFileManager(&fileManager);
    

    TitleBar titleBar;
    createTitleBar(&titleBar, screenWidth, screenHeight);

    Rectangle currentZeroPosition = {DEFAULT_PADDING, titleBar.height + DEFAULT_PADDING, screenWidth - DEFAULT_PADDING * 2, screenHeight - titleBar.height - DEFAULT_PADDING * 2};

    SetConfigFlags( FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    InitWindow(screenWidth, screenHeight, "raygui - controls test suite");
    // ----------------------------------------------------------------------------------------

    // initialization
    // ----------------------------------------------------------------------------------------

    Navbar navbar;
    createNavbar(&navbar);

    Toolbar toolbar;
    createToolbar(&toolbar);

    Sidebar sidebar;
    createSidebar(&sidebar);

    Body body;
    createBody(&body);

    while (!titleBar.exitWindow && !WindowShouldClose())
    {
        if (IsWindowResized() || IsWindowMaximized)
        {
            screenWidth = GetScreenWidth();
            screenHeight = GetScreenHeight();
            currentZeroPosition = (Rectangle) {DEFAULT_PADDING, titleBar.height + DEFAULT_PADDING, screenWidth - DEFAULT_PADDING * 2, screenHeight - titleBar.height - DEFAULT_PADDING * 2};
        }

        // Update
        //----------------------------------------------------------------------------------
        updateTitleBar(&titleBar);

        updateNavbar(&navbar, currentZeroPosition);

        updateToolbar(&toolbar, currentZeroPosition);

        updateSidebar(&sidebar, currentZeroPosition);

        updateBody(&body, currentZeroPosition);
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        drawTitleBar(&titleBar);

        drawBody(&body);

        drawSidebar(&sidebar);

        drawToolbar(&toolbar);

        drawNavbar(&navbar);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
