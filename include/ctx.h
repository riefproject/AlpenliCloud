#ifndef CTX_H
#define CTX_H

#include "raylib.h"

/**
 * @file ctx.h
 * @brief Application context management for GUI components and state
 * @author AlpenliCloud Development Team
 * @date 2025
 */

 /*
 ====================================================================
     FORWARD DECLARATIONS
 ====================================================================
 */

typedef struct FileManager FileManager;
typedef struct Body Body;
typedef struct Navbar Navbar;
typedef struct Sidebar Sidebar;
typedef struct TitleBar TitleBar;
typedef struct Toolbar Toolbar;
typedef struct Footer Footer;

/*
====================================================================
    DATA STRUCTURES
====================================================================
*/

/**
 * @brief Main application context structure
 *
 * Contains all GUI components and application state information.
 * Acts as the central hub for managing the entire application's
 * user interface and interactions.
 */
typedef struct Context {
    FileManager* fileManager;       /**< File manager component for file operations */
    Body* body;                     /**< Main content area component */
    Navbar* navbar;                 /**< Navigation bar component */
    Sidebar* sidebar;               /**< Sidebar component for quick access */
    TitleBar* titleBar;             /**< Title bar component */
    Toolbar* toolbar;               /**< Toolbar component with action buttons */
    Footer* footer;                 /**< Footer component for status information */

    Rectangle* currentZeroPosition; /**< Current zero position rectangle for layout */

    int height;                     /**< Current window height */
    int width;                      /**< Current window width */

    bool disableGroundClick;        /**< Flag to disable background click events */
} Context;

/*
====================================================================
    CONTEXT MANAGEMENT
====================================================================
*/

/**
 * @brief Initializes application context with all GUI components
 *
 * Creates and initializes the main application context with all necessary
 * GUI components. Allocates memory for all components and sets up the
 * initial layout based on screen dimensions.
 *
 * @param[out] ctx Context structure to be initialized
 * @param[in] fileManager File manager instance to associate with context
 * @param[in] screenWidth Initial screen width
 * @param[in] screenHeight Initial screen height
 *
 * @pre ctx must be a valid pointer to Context structure
 * @pre fileManager must be a valid initialized FileManager instance
 * @pre screenWidth and screenHeight must be positive values
 * @post Context is fully initialized with allocated GUI components
 * @post currentZeroPosition is allocated based on DEFAULT_PADDING
 * @post All GUI components (titleBar, toolbar, navbar, sidebar, body) are allocated and initialized
 * @post disableGroundClick is set to false
 *
 * @note Memory allocation is performed for all GUI components
 * @note Screen dimensions are stored in context for layout calculations
 * @warning Ensure proper cleanup of allocated memory when context is no longer needed
 *
 * @see updateContext() for updating context after screen changes
 *
 * @author Farras
 */
void createContext(Context* ctx, FileManager* fileManager, int screenWidth, int screenHeight);

/**
 * @brief Updates context with current screen dimensions and refreshes all components
 *
 * Updates the application context to reflect current screen dimensions and
 * refreshes all GUI components. Should be called when screen size changes
 * or when components need to be synchronized.
 *
 * @param[in,out] ctx Context structure to be updated
 * @param[in] fileManager File manager instance to associate with context
 *
 * @pre ctx must be a valid initialized Context structure
 * @pre fileManager must be a valid initialized FileManager instance
 * @post currentZeroPosition is updated based on current screen dimensions and titleBar height
 * @post Context width and height are updated to current screen dimensions
 * @post fileManager reference is updated
 * @post All GUI components are updated in sequence: titleBar → navbar → toolbar → sidebar → body
 *
 * @note Uses GetScreenWidth() and GetScreenHeight() to get current dimensions
 * @note Component update order is important for proper layout calculation
 * @note Should be called in the main update loop for responsive design
 *
 * @see createContext() for initial context creation
 *
 * @author Farras
 */
void updateContext(Context* ctx, FileManager* fileManager);

#endif