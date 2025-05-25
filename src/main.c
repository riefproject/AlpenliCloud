#define RAYGUI_IMPLEMENTATION

#include "macro.h"
#include "raygui.h"
#include "raylib.h"
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

#include "file_manager.h"
#include "utils.h"
#include "gui/body.h"
#include "gui/navbar.h"
#include "gui/sidebar.h"
#include "gui/titlebar.h"
#include "gui/toolbar.h"
// #include "nbtree.h"

// int main(){
//     // MakeDirectory(".dir/baru");
// }

int main() {
    // Windows config
    // ----------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    FileManager fileManager;

    createFileManager(&fileManager);

    initFileManager(&fileManager);

    // createFile(&fileManager, ITEM_FILE, ".dir/root", "INI FILE BARU DIBUAT.txt");
    // printTree((fileManager.treeCursor), 0);
    // printf("Enter to Undo...");
    // getchar();
    // undo(&fileManager);
    // printf("Enter to redo...");
    // getchar();
    // redo(&fileManager);


    // return 0;

    TitleBar titleBar;
    createTitleBar(&titleBar, screenWidth, screenHeight);

    Rectangle currentZeroPosition = { DEFAULT_PADDING, titleBar.height + DEFAULT_PADDING, screenWidth - DEFAULT_PADDING * 2, screenHeight - titleBar.height - DEFAULT_PADDING * 2 };
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    InitWindow(screenWidth, screenHeight, "AlpenliCloud");
    SetWindowIcon(LoadImage("resources/icon.png"));
    // ----------------------------------------------------------------------------------------

    // initialization
    // ----------------------------------------------------------------------------------------

    Navbar navbar;
    createNavbar(&navbar);


    Sidebar sidebar;
    createSidebar(&sidebar);

    Toolbar toolbar;
    createToolbar(&toolbar, &fileManager, &sidebar);

    Body body;
    createBody(&body);



    while (!titleBar.exitWindow && !WindowShouldClose()) {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        currentZeroPosition = (Rectangle){ DEFAULT_PADDING, titleBar.height + DEFAULT_PADDING, screenWidth - DEFAULT_PADDING * 2, screenHeight - titleBar.height - DEFAULT_PADDING * 2 };

        // Update
        //----------------------------------------------------------------------------------
        updateTitleBar(&titleBar);

        updateNavbar(&navbar, currentZeroPosition, &fileManager);

        updateToolbar(&toolbar, currentZeroPosition);

        updateSidebar(&sidebar, currentZeroPosition, &fileManager);

        updateBody(&body, currentZeroPosition, &fileManager);

        ShortcutKeys(&toolbar, &navbar);
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

    exit(1);

    CloseWindow();
    return 0;
}
