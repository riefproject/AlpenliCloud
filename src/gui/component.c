#include "gui/component.h"
#include "raylib.h"
#include "raygui.h"

bool GuiButtonTooltip(Rectangle bounds, const char *text, const char *tooltip)
{
    bool pressed = GuiButton(bounds, text);

    if (CheckCollisionPointRec(GetMousePosition(), bounds))
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

    return pressed;
}

