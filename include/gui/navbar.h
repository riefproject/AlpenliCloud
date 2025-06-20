#ifndef NAVBAR_H
#define NAVBAR_H

#include <stdbool.h>
#include "raylib.h"
#include "file_manager.h"
#include "macro.h"

/**
 * @file navbar.h
 * @brief Navigation bar component for file manager GUI
 * @author AlpenliCloud Development Team
 * @date 2025
 */

 /*
 ====================================================================
     FORWARD DECLARATIONS AND DATA STRUCTURES
 ====================================================================
 */

typedef struct Context Context;

/**
 * @brief Navigation bar component structure
 *
 * Contains all state and properties for the navigation bar component,
 * including path and search textboxes, button states, and layout information.
 */
typedef struct Navbar {
    bool textboxPatheditMode;              /**< Flag indicating if path textbox is in edit mode */
    char textboxPath[MAX_STRING_LENGTH];   /**< Path textbox content buffer */

    bool textboxSearcheditMode;            /**< Flag indicating if search textbox is in edit mode */
    char textboxSearch[MAX_STRING_LENGTH]; /**< Search textbox content buffer */

    bool shouldGoToPath;                   /**< Flag indicating if navigation to path should occur */
    bool shouldSearch;                     /**< Flag indicating if search should be performed */

    bool isUndoButtonClicked;              /**< Flag indicating if undo button was clicked */
    bool isRedoButtonClicked;              /**< Flag indicating if redo button was clicked */
    bool isGoBackButtonClicked;            /**< Flag indicating if go back button was clicked */

    Rectangle currentZeroPosition;         /**< Current layout position and dimensions */

    Context* ctx;                          /**< Reference to application context */
} Navbar;

/*
====================================================================
    NAVBAR INITIALIZATION AND UPDATE
====================================================================
*/

/**
 * @brief Creates and initializes navbar component
 *
 * Initializes the Navbar structure with default values and context reference.
 * Sets all flags to false and clears textbox contents.
 *
 * @param[out] navbar Navbar structure to be initialized
 * @param[in] ctx Application context reference
 *
 * @pre navbar must be a valid pointer to Navbar structure
 * @pre ctx must be a valid initialized Context structure
 * @post Navbar is initialized with all button flags set to false
 * @post shouldGoToPath and shouldSearch flags are set to false
 * @post textboxPath and textboxSearch are cleared
 * @post editMode flags are set to false
 * @post currentZeroPosition is initialized to empty
 * @post ctx reference is stored
 *
 * @note All string buffers are initialized to empty strings
 * @warning Ensure ctx remains valid for the lifetime of the navbar
 *
 * @see updateNavbar() for navbar state updates
 *
 * @author Farras
 */
void createNavbar(Navbar* navbar, Context* ctx);

/**
 * @brief Updates navbar component state and handles interactions
 *
 * Updates the navbar state and processes button interactions and manual path navigation.
 * Handles path navigation, search functionality, and button click events.
 *
 * @param[in,out] navbar Navbar structure to be updated
 * @param[in] ctx Application context reference
 *
 * @pre navbar must be a valid initialized Navbar structure
 * @pre ctx must be a valid Context structure with accessible file manager
 * @post currentZeroPosition is updated based on current layout
 * @post Manual path navigation is processed using searchTree and goTo operations
 * @post Search string is processed for file filtering
 * @post Undo/redo/goBack button states are handled
 * @post textboxPath is synchronized with currentPath when not in edit mode
 *
 * @note Uses file manager's searchTree and goTo functions for navigation
 * @note Search functionality filters items based on search string
 * @note Path textbox automatically syncs with current directory when not editing
 *
 * @see createNavbar() for navbar initialization
 * @see drawNavbar() for navbar rendering
 *
 * @author Farras
 */
void updateNavbar(Navbar* navbar, Context* ctx);

/*
====================================================================
    RENDERING AND DRAWING
====================================================================
*/

/**
 * @brief Renders the navbar component
 *
 * Draws the navbar with undo/redo/back buttons, path textbox, search textbox,
 * and separator lines. Layout is responsive based on screen width.
 *
 * @param[in] navbar Navbar structure with valid currentZeroPosition
 *
 * @pre navbar must be a valid Navbar structure
 * @pre currentZeroPosition must be properly set
 * @post Undo/redo/back buttons are drawn using GuiButtonCustom with icons (#56#/#57#/#117#)
 * @post Path textbox is drawn with folder icon (#1#)
 * @post Search textbox is drawn with search icon (#42#) and "Search Item" placeholder
 * @post GuiLine is drawn as separator
 * @post Layout is responsive based on screen width
 *
 * @note Uses custom GUI components for consistent styling
 * @note Icons are referenced by their numeric identifiers
 * @note Responsive layout adjusts to different screen sizes
 *
 * @see updateNavbar() for navbar state management
 *
 * @author Farras
 */
void drawNavbar(Navbar* navbar);

#endif // NAVBAR_H