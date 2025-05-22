#include "gui/component.h"
#include "raylib.h"
#include "raygui.h"
#include "macro.h"

bool GuiButtonCustom(Rectangle bounds, const char *text, const char *tooltip, bool disabled)
{
    if (disabled) GuiDisable();
    
    bool pressed = GuiButton(bounds, text);

    if (CheckCollisionPointRec(GetMousePosition(), bounds) && !disabled)
    {
        int padding = 5;
        int fontSize = GuiGetStyle(DEFAULT, TEXT_SIZE);

        int textWidth = MeasureText(tooltip, fontSize);
        int tooltipWidth = textWidth + 2 * padding;
        int tooltipHeight = fontSize + 2 * padding;

        Vector2 tooltipPos = {
            bounds.x,
            bounds.y + bounds.height + 5
        };

        DrawRectangleRec(
            (Rectangle){ tooltipPos.x, tooltipPos.y, tooltipWidth, tooltipHeight },
            Fade(GetColor(GuiGetStyle(DEFAULT, BASE_COLOR_NORMAL)), 0.95f)
        );

        DrawRectangleLinesEx(
            (Rectangle){ tooltipPos.x, tooltipPos.y, tooltipWidth, tooltipHeight },
            1,
            GetColor(GuiGetStyle(DEFAULT, BORDER_COLOR_NORMAL))
        );

        DrawText(
            tooltip,
            tooltipPos.x + padding,
            tooltipPos.y + padding,
            fontSize,
            GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL))
        );
    }

    GuiEnable();

    return pressed;
}

bool GuiTextBoxCustom(Rectangle bounds, const char *placeholder, char *inputText, int textSize, bool *editMode, bool disabled)
{
    bool pressedEnter = false;

    if (disabled) GuiDisable();

    if (GuiTextBox(bounds, inputText, textSize, *editMode)) *editMode = !*editMode;

    if (*editMode && IsKeyPressed(KEY_ENTER))
    {
        pressedEnter = true;
        *editMode = false;
    }

    if (!*editMode && inputText[0] == '\0') {
        GuiLabel((Rectangle){bounds.x + TINY_PADING, bounds.y + bounds.height / 2, 20}, placeholder);
    }

    if (disabled) GuiEnable();

    return pressedEnter;
}
