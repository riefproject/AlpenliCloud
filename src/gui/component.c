#include "gui/component.h"
#include "item.h"
#include "macro.h"
#include "raygui.h"
#include "raylib.h"

#include <string.h>
#include <stdlib.h>

bool GuiNewButton(NewButtonProperty* buttonProperty) {
    bool itemCreated = false;

    // Tombol utama
    if (GuiButtonCustom(
        buttonProperty->btnRect,
        buttonProperty->placeholder,
        buttonProperty->tooltip,
        buttonProperty->disabled)) {
        buttonProperty->dropdownActive = !buttonProperty->dropdownActive;
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
            *buttonProperty->showModal = true;
            buttonProperty->dropdownActive = false;
            strcpy(buttonProperty->inputBuffer, "");
            *buttonProperty->inputEditMode = true;
        }

        // Tombol Folder
        Rectangle folderBtn = {
            fileBtn.x,
            fileBtn.y + btnHeight + DEFAULT_PADDING,
            fileBtn.width,
            btnHeight };

        if (GuiButton(folderBtn, "#204# Folder")) {
            buttonProperty->selectedType = ITEM_FOLDER;
            *buttonProperty->showModal = true;
            buttonProperty->dropdownActive = false;
            strcpy(buttonProperty->inputBuffer, "");
            *buttonProperty->inputEditMode = true;
        }

        // Tutup dropdown jika klik di luar area dropdown & tombol
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
            !CheckCollisionPointRec(mouse, dropdownBg) &&
            !CheckCollisionPointRec(mouse, buttonProperty->btnRect)) {
            buttonProperty->dropdownActive = false;
        }
    }

    if (buttonProperty->showModal) {
        // Selalu update modal di tengah layar
        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();
        buttonProperty->modalRect.x = (screenW - buttonProperty->modalRect.width) / 2;
        buttonProperty->modalRect.y = (screenH - buttonProperty->modalRect.height) / 2;
        bool quit = false;

        const char* typeStr = (buttonProperty->selectedType == ITEM_FILE) ? "File" : "Folder";
        char title[64];
        snprintf(title, sizeof(title), "Create New %s", typeStr);

        DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.3f));

        quit = GuiWindowBox(buttonProperty->modalRect, title);

        // Input box
        Rectangle inputBox = {
            buttonProperty->modalRect.x + 20,
            buttonProperty->modalRect.y + 45,
            buttonProperty->modalRect.width - 40,
            30 };

        if (GuiTextBoxCustom(inputBox, NULL, "Enter name...",
            buttonProperty->inputBuffer,
            MAX_STRING_LENGTH,
            buttonProperty->inputEditMode,
            false)) {
            if (strcmp(buttonProperty->inputBuffer, "") != 0) {
                itemCreated = true;
                buttonProperty->showModal = false;
                buttonProperty->inputEditMode = false;
            }
        }

        // Tombol Create dan Cancel
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

        if (GuiButton(btnCreate, "Create") && strcmp(buttonProperty->inputBuffer, "") != 0) {
            itemCreated = true;
            buttonProperty->showModal = false;
            buttonProperty->inputEditMode = false;
        }

        if (GuiButton(btnCancel, "Cancel") || quit) {
            buttonProperty->showModal = false;
            buttonProperty->inputEditMode = false;
            strcpy(buttonProperty->inputBuffer, "");
        }
    }

    return itemCreated;
}

bool GuiButtonCustom(Rectangle bounds, const char* text, const char* tooltip, bool disabled) {
    if (disabled)
        GuiDisable();

    bool pressed = GuiButton(bounds, text);

    if (CheckCollisionPointRec(GetMousePosition(), bounds) && !disabled) {
        int padding = 5;
        int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);

        int textWidth = MeasureText(tooltip, fontSize);
        int tooltipWidth = textWidth + 2 * padding;
        int tooltipHeight = fontSize + 2 * padding;

        Vector2 tooltipPos = {
            bounds.x,
            bounds.y + bounds.height + 5 };

        DrawRectangleRec(
            (Rectangle) {
            tooltipPos.x, tooltipPos.y, tooltipWidth, tooltipHeight
        },
            Fade(GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL)), 0.95f));

        DrawRectangleLinesEx(
            (Rectangle) {
            tooltipPos.x, tooltipPos.y, tooltipWidth, tooltipHeight
        },
            1,
            GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL)));

        DrawText(
            tooltip,
            tooltipPos.x + padding,
            tooltipPos.y + padding,
            fontSize,
            GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
    }

    GuiEnable();

    return pressed;
}

bool GuiTextBoxCustom(Rectangle bounds, char* icon, char* placeholder, char* inputText, int textSize, bool* editMode, bool disabled) {
    bool pressedEnter = false;

    if (disabled)
        GuiDisable();

    float textOffsetX = bounds.x + TINY_PADDING;
    if (icon != NULL && icon[0] != '\0') {
        GuiLabel((Rectangle) { bounds.x + TINY_PADDING, bounds.y + bounds.height / 2 - 10, bounds.width, 20 }, icon);
        textOffsetX += 24;
    }

    if (*editMode) {
        if (GuiTextBox(bounds, inputText, textSize, true)) {
            *editMode = false;
        }

        if (IsKeyPressed(KEY_ENTER)) {
            pressedEnter = true;
            *editMode = false;
        }
    }
    else {
        DrawRectangleLinesEx(bounds, 1, Fade(GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)), 0.6f));
        if (inputText[0] == '\0') {
            GuiLabel((Rectangle) { textOffsetX, bounds.y + bounds.height / 2 - 10, bounds.width, 20 }, placeholder);
        }
        else {
            GuiLabel((Rectangle) { textOffsetX, bounds.y + bounds.height / 2 - 10, bounds.width, 20 }, inputText);
        }

        if (CheckCollisionPointRec(GetMousePosition(), bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            *editMode = true;
        }
    }

    if (disabled)
        GuiEnable();

    return pressedEnter;
}
