#include "gui/component.h"
#include "ctx.h"
#include "gui/toolbar.h"

#include "item.h"
#include "macro.h"
#include "raygui.h"
#include "raylib.h"

#include "gui/sidebar.h"
#include <stdlib.h>
#include <string.h>

void GuiNewButton(ButtonWithModalProperty* buttonProperty, Context* ctx) {
    bool itemCreated = false;

    // Tombol utama
    if (GuiButtonCustom(
        buttonProperty->btnRect,
        buttonProperty->placeholder,
        buttonProperty->tooltip,
        buttonProperty->disabled, ctx->disableGroundClick)) {
        buttonProperty->dropdownActive = !buttonProperty->dropdownActive;
        ctx->disableGroundClick = true;
        buttonProperty->showModal = false;
    }

    if (buttonProperty->dropdownActive) {
        // Ukuran tombol (tetap seperti btnRect)
        float btnWidth = buttonProperty->btnRect.width + 50;
        float btnHeight = buttonProperty->btnRect.height;

        // Total tinggi dropdown termasuk padding: 2 tombol + 3x padding (atas, antar tombol, bawah)
        float totalHeight = (btnHeight * 2) + (DEFAULT_PADDING * 3);

        // Area latar dropdown (latar luar)
        Rectangle dropdownBg = {
            buttonProperty->btnRect.x,
            buttonProperty->btnRect.y + btnHeight + 2,
            btnWidth,
            totalHeight };

        // Gambar latar belakang dan border
        DrawRectangleRec(dropdownBg, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
        DrawRectangleLinesEx(dropdownBg, 1, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

        // Tombol File
        Rectangle fileBtn = {
            dropdownBg.x + DEFAULT_PADDING,
            dropdownBg.y + DEFAULT_PADDING,
            btnWidth - 2 * DEFAULT_PADDING,
            btnHeight };

        if (GuiButton(fileBtn, "#8# File")) {
            buttonProperty->selectedType = ITEM_FILE;
            buttonProperty->showModal = true;
            buttonProperty->dropdownActive = false;
            ctx->disableGroundClick = true;
            strcpy(buttonProperty->inputBuffer, "");
            buttonProperty->inputEditMode = true;
        }

        // Tombol Folder
        Rectangle folderBtn = {
            fileBtn.x,
            fileBtn.y + btnHeight + DEFAULT_PADDING,
            fileBtn.width,
            btnHeight };

        if (GuiButton(folderBtn, "#204# Folder")) {
            buttonProperty->selectedType = ITEM_FOLDER;
            buttonProperty->showModal = true;
            buttonProperty->dropdownActive = false;
            ctx->disableGroundClick = false;
            strcpy(buttonProperty->inputBuffer, "");
            buttonProperty->inputEditMode = true;
        }

        // Tutup dropdown jika klik di luar area dropdown & tombol
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            !CheckCollisionPointRec(mouse, dropdownBg) &&
            !CheckCollisionPointRec(mouse, buttonProperty->btnRect)) {
            buttonProperty->dropdownActive = false;
            ctx->disableGroundClick = false;
        }
    }
}

void DrawCreateModal(Context *ctx, ButtonWithModalProperty *buttonProperty) {
    if (!buttonProperty->showModal)
        return;

    ctx->disableGroundClick = true;

    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    buttonProperty->modalRect.x = (screenW - buttonProperty->modalRect.width) / 2;
    buttonProperty->modalRect.y = (screenH - buttonProperty->modalRect.height) / 2;

    DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.3f));

    const char* typeStr = (buttonProperty->selectedType == ITEM_FILE) ? "File" : "Folder";
    char title[64];
    snprintf(title, sizeof(title), "%s %s", buttonProperty->title, typeStr);

    bool quit = GuiWindowBox(buttonProperty->modalRect, title);

    Rectangle inputBox = {
        buttonProperty->modalRect.x + 20,
        buttonProperty->modalRect.y + 45,
        buttonProperty->modalRect.width - 40,
        30 };

    if (GuiTextBoxCustom(inputBox, NULL, "Enter name...",
        buttonProperty->inputBuffer,
        MAX_STRING_LENGTH,
        &buttonProperty->inputEditMode,
        false, false)) {
        if (strcmp(buttonProperty->inputBuffer, "") != 0) {
            buttonProperty->itemCreated = true;
            buttonProperty->showModal = false;
            ctx->disableGroundClick = false;
            buttonProperty->inputEditMode = false;
        }
    }

    Rectangle btnCreate = {
        buttonProperty->modalRect.x + 20,
        buttonProperty->modalRect.y + buttonProperty->modalRect.height - 45,
        (buttonProperty->modalRect.width - 50) / 2,
        30 };

    Rectangle btnCancel = {
        btnCreate.x + btnCreate.width + 10,
        btnCreate.y,
        btnCreate.width,
        30 };

    if ((GuiButton(btnCreate, buttonProperty->yesButtonText) || IsKeyPressed(KEY_ENTER)) && strcmp(buttonProperty->inputBuffer, "") != 0) {
        buttonProperty->itemCreated = true;
        buttonProperty->showModal = false;
        ctx->disableGroundClick = false;
        buttonProperty->inputEditMode = false;
    }

    if (GuiButton(btnCancel, buttonProperty->noButtonText) || quit) {
        buttonProperty->itemCreated = false;
        buttonProperty->showModal = false;
        buttonProperty->inputEditMode = false;
        ctx->disableGroundClick = false;
        strcpy(buttonProperty->inputBuffer, "");
    }
}

bool GuiButtonCustom(Rectangle bounds, const char* text, const char* tooltip, bool disabled, bool notClickable) {
    bool pressed = false;
    // printf("disabled: %d, notClickable: %d\n", disabled, notClickable);
    if (disabled || notClickable) {
        // Warna sesuai style
        Color borderColor = GetColor(GuiGetStyle(BUTTON, BORDER_COLOR_NORMAL));
        Color baseColor = GetColor(GuiGetStyle(BUTTON, BASE_COLOR_NORMAL));
        Color textColor = GetColor(GuiGetStyle(BUTTON, TEXT_COLOR_NORMAL));

        if (disabled) {
            borderColor.a = 100;
            baseColor.a = 100;
            textColor.a = 100;
        }

        DrawRectangleRec(bounds, baseColor);
        DrawRectangleLinesEx(bounds, 2, borderColor);

        GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
        GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(textColor));
        GuiLabel(bounds, text);
    }
    else {
        pressed = GuiButton(bounds, text);

        if (tooltip && CheckCollisionPointRec(GetMousePosition(), bounds)) {
            int padding = 5;
            int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);

            int textWidth = MeasureText(tooltip, fontSize);
            int tooltipWidth = textWidth + 2 * padding;
            int tooltipHeight = fontSize + 2 * padding;

            Vector2 tooltipPos = { bounds.x, bounds.y + bounds.height + 5 };

            int screenWidth = GetScreenWidth();
            if (tooltipPos.x + tooltipWidth > screenWidth) {
                tooltipPos.x = bounds.x + bounds.width - tooltipWidth;
            }

            DrawRectangleRec((Rectangle) { tooltipPos.x, tooltipPos.y, tooltipWidth, tooltipHeight }, Fade(GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL)), 0.95f));
            DrawRectangleLinesEx((Rectangle) { tooltipPos.x, tooltipPos.y, tooltipWidth, tooltipHeight }, 1, GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));
            DrawText(tooltip, tooltipPos.x + padding, tooltipPos.y + padding, fontSize, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
        }
    }

    GuiSetStyle(LABEL, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiSetState(STATE_NORMAL);
    GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL))));
    return pressed;
}

bool GuiTextBoxCustom(Rectangle bounds, char* icon, char* placeholder, char* inputText, int textSize, bool* editMode, bool disabled, bool notClickable) {
    bool pressedEnter = false;

    if (disabled || notClickable) {
        GuiDisable();
        if (notClickable) {
            disabled = true;
            GuiSetState(STATE_NORMAL);
        }
    }

    float textOffsetX = bounds.x + TINY_PADDING;

    // Gambar ikon jika ada
    if (icon != NULL && icon[0] != '\0') {
        GuiLabel((Rectangle) { bounds.x + TINY_PADDING, bounds.y + bounds.height / 2 - 10, bounds.width, 20 }, icon);
        textOffsetX += 24;
    }

    // Jika sedang dalam edit mode dan diperbolehkan interaksi
    if (*editMode && !notClickable && !disabled) {
        if (GuiTextBox(bounds, inputText, textSize, true)) {
            *editMode = false;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            pressedEnter = true;
            *editMode = false;
        }
    }
    else {
        // Mode tampilan (readonly)
        DrawRectangleLinesEx(bounds, 1, Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), 0.6f));

        Rectangle labelBounds = {
            textOffsetX,
            bounds.y + bounds.height / 2 - 10,
            bounds.width - (textOffsetX - bounds.x),
            20 };

        if (inputText[0] == '\0') {
            GuiLabel(labelBounds, placeholder);
        }
        else {
            GuiLabel(labelBounds, inputText);
        }

        // Mencegah masuk ke edit mode saat tidak boleh diklik
        if (!disabled && !notClickable &&
            CheckCollisionPointRec(GetMousePosition(), bounds) &&
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            *editMode = true;
        }
    }

    if (disabled || notClickable)
        GuiSetState(STATE_NORMAL);

    return (!notClickable && !disabled) ? pressedEnter : false;
}

static ProgressBarState progressState = { false, false };

void showPasteProgressBar(int currentProgress, int totalItems, const char* currentItemName) {
    if (!progressState.isActive) {
        progressState.isActive = true;
        progressState.shouldCancel = false;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Hitung persentase
    float progressValue = (float)currentProgress / (float)totalItems;

    // Setup dimensi mengikuti style modal yang ada
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int modalWidth = 450;
    int modalHeight = 200;

    Rectangle modalRect = { (screenWidth - modalWidth) / 2.0f,(screenHeight - modalHeight) / 2.0f,modalWidth,modalHeight };

    // Background overlay seperti di DrawCreateModal
    DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.3f));

    // Window box seperti style yang ada
    const char* title = "Pasting Files...";
    bool quit = GuiWindowBox(modalRect, title);

    if (quit) {
        progressState.shouldCancel = true;
    }

    // Progress info text
    Rectangle infoRect = { modalRect.x + 20,modalRect.y + 45,modalRect.width - 40,20 };

    char infoText[256];
    snprintf(infoText, sizeof(infoText), "Processing item %d of %d", currentProgress + 1, totalItems);
    GuiLabel(infoRect, infoText);

    // Progress bar
    Rectangle progressRect = { modalRect.x + 20,modalRect.y + 75,modalRect.width - 40,25 };

    GuiProgressBar(progressRect, NULL, TextFormat("%.1f%%", progressValue * 100), &progressValue, 0.0f, 1.0f);

    // Current item name
    Rectangle itemNameRect = { modalRect.x + 20,modalRect.y + 110,modalRect.width - 40,20 };

    if (currentItemName && strlen(currentItemName) > 0) {
        char displayName[60];
        if (strlen(currentItemName) > 55) {
            strncpy(displayName, currentItemName, 52);
            displayName[52] = '.';
            displayName[53] = '.';
            displayName[54] = '.';
            displayName[55] = '\0';
        }
        else {
            strcpy(displayName, currentItemName);
        }

        GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(GRAY));
        GuiLabel(itemNameRect, displayName);
        GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL))));
    }

    // Cancel button seperti style button di modal
    Rectangle cancelBtn = { modalRect.x + modalRect.width - 90,modalRect.y + modalRect.height - 45,70,30 };

    if (GuiButton(cancelBtn, "Cancel")) {
        progressState.shouldCancel = true;
    }

    EndDrawing();
}

bool shouldCancelPaste() {
    return progressState.shouldCancel;
}

void resetProgressBarState() {
    progressState.shouldCancel = false;
    progressState.isActive = false;
}