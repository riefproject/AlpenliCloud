#include "ctx.h"
#include "file_manager.h"
#include "gui/body.h"
#include "gui/navbar.h"
#include "gui/sidebar.h"
#include "gui/titlebar.h"
#include "gui/toolbar.h"
#include "macro.h"

#include <stdlib.h>

void createContext(Context* ctx, FileManager* fileManager, int screenWidth, int screenHeight) {
    if (!ctx)
        return;

    ctx->fileManager = fileManager;
    fileManager->ctx = ctx;
    
    ctx->width = screenWidth;
    ctx->height = screenHeight;
    ctx->disableGroundClick = false;

    // Alokasi currentZeroPosition
    ctx->currentZeroPosition = (Rectangle*)malloc(sizeof(Rectangle));
    if (ctx->currentZeroPosition) {
        *ctx->currentZeroPosition = (Rectangle){
            DEFAULT_PADDING,
            DEFAULT_PADDING,
            screenWidth - DEFAULT_PADDING * 2,
            screenHeight - DEFAULT_PADDING * 2 };
    }

    // Alokasi dan inisialisasi komponen GUI
    ctx->titleBar = (TitleBar*)malloc(sizeof(TitleBar));
    ctx->toolbar = (Toolbar*)malloc(sizeof(Toolbar));
    ctx->navbar = (Navbar*)malloc(sizeof(Navbar));
    ctx->sidebar = (Sidebar*)malloc(sizeof(Sidebar));
    ctx->body = (Body*)malloc(sizeof(Body));

    if (ctx->titleBar)
        createTitleBar(ctx->titleBar, ctx);
    if (ctx->toolbar)
        createToolbar(ctx->toolbar, ctx);
    if (ctx->navbar)
        createNavbar(ctx->navbar, ctx);
    if (ctx->sidebar)
        createSidebar(ctx->sidebar, ctx);
    if (ctx->body)
        createBody(ctx, ctx->body);
}

void updateContext(Context* ctx, FileManager* fileManager) {
    if (!ctx || !ctx->currentZeroPosition || !ctx->titleBar)
        return;

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    *ctx->currentZeroPosition = (Rectangle){
        DEFAULT_PADDING,
        ctx->titleBar->height + DEFAULT_PADDING,
        screenWidth - DEFAULT_PADDING * 2,
        screenHeight - ctx->titleBar->height - DEFAULT_PADDING * 2 };

    ctx->width = screenWidth;
    ctx->height = screenHeight;
    ctx->fileManager = fileManager;
    fileManager->ctx = ctx;

    updateTitleBar(ctx->titleBar, ctx);
    updateNavbar(ctx->navbar, ctx);
    updateToolbar(ctx->toolbar, ctx);
    updateSidebar(ctx->sidebar, ctx);
    updateBody(ctx, ctx->body);
}
