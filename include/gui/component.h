#ifndef COMPONENT_H
#define COMPONENT_H

#define DEFAULT_PADDING 10
#define TINY_PADDING 5

#include "raylib.h"

bool GuiButtonTooltip(Rectangle bounds, const char *text, const char *tooltip);

#endif // COMPONENT_H