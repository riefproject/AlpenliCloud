#ifndef UTILS_H
#define UTILS_H 
typedef struct Toolbar Toolbar;
typedef struct Navbar Navbar;
typedef struct Body Body;
void inputString(char** s);
void trimTrailingSlash(char *path);
void ShortcutKeys(Toolbar* toolbar, Navbar* navbar, Body* body);
#endif