#define RAYGUI_IMPLEMENTATION

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "body.h"
#include "component.h"
#include "file_manager.h"
#include "footer.h"
#include "macro.h"
#include "navbar.h"
#include "raygui.h"
#include "raylib.h"
#include "sidebar.h"
#include "titlebar.h"
#include "toolbar.h"
#include "utils.h"

#include "ctx.h"

#include "gui/rheme.h"
// Fungsi untuk membaca isi direktori
void readDirectory(const char *path, FileManager *fileManager) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        printf("Error: Unable to open directory %s\n", path);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("Found: %s\n", entry->d_name);
            // Tambahkan logika untuk memperbarui fileManager atau struktur data sidebar
        }
    }

    closedir(dir);
}

// Fungsi untuk memeriksa perubahan direktori
int checkDirectoryChanges(const char *path, FileManager *fileManager) {
    static time_t lastModified = 0;
    struct stat dirStat;

    if (stat(path, &dirStat) == -1) {
        printf("Error: Unable to stat directory %s\n", path);
        return 0;
    }

    if (dirStat.st_mtime != lastModified) {
        lastModified = dirStat.st_mtime;
        readDirectory(path, fileManager);
        return 1; // Ada perubahan
    }

    return 0; // Tidak ada perubahan
}

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
    // printf("%s\n", fileManager.currentPath);
    // printf("Enter to Undo...");
    // getchar();
    // undo(&fileManager);
    // printf("Enter to redo...");
    // getchar();
    // redo(&fileManager);

    // printTrash(fileManager.trash);

    // return 0;

    Context ctx;
    createContext(&ctx, &fileManager, screenWidth, screenHeight);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_UNDECORATED | FLAG_VSYNC_HINT);
    SetTargetFPS(60);

    InitWindow(screenWidth, screenHeight, "AlpenliCloud");
    SetWindowIcon(LoadImage("assets/icon.png"));
    // ----------------------------------------------------------------------------------------

    // initialization
    // ----------------------------------------------------------------------------------------
    // createTitleBar(ctx.titleBar, &ctx);
    // createNavbar(ctx.navbar, &ctx);
    // createSidebar(ctx.sidebar, &ctx);
    // createToolbar(ctx.toolbar, &ctx);
    // createBody(&ctx, ctx.body);

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

        drawFooter(ctx.footer);

        drawNavbar(ctx.navbar);

        DrawNewItemModal(&ctx);
        DrawRenameItemModal(&ctx);
        DrawImportModal(&ctx);

        EndDrawing();
    }

    saveTrashToFile(&fileManager);
    CloseWindow();
    return 0;
}
