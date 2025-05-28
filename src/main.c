#define RAYGUI_IMPLEMENTATION

#include "macro.h"
#include "raygui.h"
#include "raylib.h"
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>

#include "file_manager.h"
#include "gui/body.h"
#include "gui/navbar.h"
#include "gui/sidebar.h"
#include "gui/titlebar.h"
#include "gui/toolbar.h"
#include "utils.h"
#include "gui/component.h"

#include "ctx.h"
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
    printTree((fileManager.treeCursor), 0);
    printf("%s\n", fileManager.currentPath);
    // printf("Enter to Undo...");
    // getchar();
    // undo(&fileManager);
    // printf("Enter to redo...");
    // getchar();
    // redo(&fileManager);

    // return 0;

    Context ctx;
    createContext(&ctx, &fileManager, screenWidth, screenHeight);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    InitWindow(screenWidth, screenHeight, "AlpenliCloud");
    SetWindowIcon(LoadImage("assets/icon.png"));
    // ----------------------------------------------------------------------------------------

    // initialization
    // ----------------------------------------------------------------------------------------
    createTitleBar(ctx.titleBar, &ctx);
    createNavbar(ctx.navbar, &ctx);
    createSidebar(ctx.sidebar, &ctx);
    createToolbar(ctx.toolbar, &ctx);
    createBody(&ctx, ctx.body);

    while (!ctx.titleBar->exitWindow && !WindowShouldClose()) {

        // Update
        //----------------------------------------------------------------------------------
        updateContext(&ctx, &fileManager);

        ShortcutKeys(&ctx);
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();


        ClearBackground(RAYWHITE);

        drawTitleBar(ctx.titleBar);

        drawBody(&ctx, ctx.body);

        drawSidebar(ctx.sidebar);

        drawToolbar(ctx.toolbar);

        drawNavbar(ctx.navbar);

        DrawCreateModal(&ctx);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
