#ifndef FOOTER_H
#define FOOTER_H

#include "raylib.h"

typedef struct Context Context;

typedef struct Footer {
    Rectangle bounds;
    Context *ctx;
    bool hover;
    Rectangle dropdownBounds;
} Footer;

void createFooter(Footer *footer, Context *ctx);
void updateFooter(Footer *footer, Context *ctx);
void drawFooter(Footer *footer);

#endif
