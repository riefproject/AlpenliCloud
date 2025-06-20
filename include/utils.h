#ifndef UTILS_H
#define UTILS_H

/**
 * @file utils.h
 * @brief Utility functions for string processing, input handling, and keyboard shortcuts
 * @author AlpenliCloud Development Team
 * @date 2025
 */

 /*
 ====================================================================
     FORWARD DECLARATIONS
 ====================================================================
 */

typedef struct Context Context;

/*
====================================================================
    INPUT AND STRING UTILITIES
====================================================================
*/

/**
 * @brief Dynamically reads a string input from user
 *
 * Reads input string from user character by character with dynamic memory allocation
 * using realloc until enter is pressed. The string is automatically resized as needed.
 *
 * @param[out] s Pointer to string pointer that will be allocated and filled
 *
 * @pre String pointer is uninitialized or empty
 * @post String is dynamically allocated, filled with user input character by character,
 *       and null-terminated
 *
 * @note Memory is allocated using realloc, caller is responsible for freeing the memory
 * @warning Input parameter must be a valid pointer to char pointer
 *
 * @author Farras
 */
void inputString(char** s);

/**
 * @brief Removes trailing slash characters from path string
 *
 * Iteratively removes all trailing '/' characters from the end of a path string
 * and adjusts the string length accordingly.
 *
 * @param[in,out] path Path string to be trimmed
 *
 * @pre Path string may contain one or more trailing '/' characters
 * @post All trailing '/' characters are removed, string length adjusted with null terminator
 *
 * @note Modifies the original string in-place
 * @warning Path parameter must be a valid null-terminated string
 *
 * @author Farras
 */
void trimTrailingSlash(char* path);

/**
 * @brief Converts string to lowercase
 *
 * Converts all characters in the string to lowercase using the tolower function.
 * The conversion is performed in-place on the original string.
 *
 * @param[in,out] str String to be converted to lowercase
 *
 * @pre String contains characters that may be mixed case
 * @post All characters in string are converted to lowercase, string remains valid
 *
 * @note Modifies the original string in-place
 * @warning String parameter must be a valid null-terminated string
 *
 * @author Farras
 */
void toLowerStr(char* str);

/*
====================================================================
    KEYBOARD SHORTCUTS AND NAVIGATION
====================================================================
*/

/**
 * @brief Handles keyboard shortcuts for file manager operations
 *
 * Processes all keyboard shortcuts for file manager operations and UI navigation
 * using raylib input handling. Supports common operations like copy, paste, cut,
 * refresh, and arrow key navigation.
 *
 * @param[in,out] ctx Context containing application state and UI components
 *
 * @pre Context contains valid application state and UI components
 * @post Keyboard shortcuts are processed (Ctrl+C/V/X for copy/paste/cut, F5 for refresh,
 *       arrow keys for navigation, etc.), UI state and file manager updated accordingly
 *
 * @note Requires raylib for input handling
 * @warning Context parameter must be a valid pointer to initialized Context structure
 *
 * @author Arief
 */
void ShortcutKeys(Context* ctx);

#endif