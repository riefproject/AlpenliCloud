#ifndef UTILS_H
#define UTILS_H
typedef struct Toolbar Toolbar;
typedef struct Navbar Navbar;
typedef struct Body Body;
typedef struct Context Context;

void inputString(char **s);
void trimTrailingSlash(char *path);
void ShortcutKeys(Context *ctx);
#endif