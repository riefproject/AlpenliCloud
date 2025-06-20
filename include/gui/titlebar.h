#ifndef TITLEBAR_H
#define TITLEBAR_H

#include "raylib.h"
#include <stdbool.h>

/**
 * @file titlebar.h
 * @brief Title bar component for window management and resizing functionality
 * @author AlpenliCloud Development Team
 * @date 2025
 */

 /*
 ====================================================================
     FORWARD DECLARATIONS AND ENUMERATIONS
 ====================================================================
 */

typedef struct Context Context;

/**
 * @brief Enumeration of window resize directions
 *
 * Defines the possible directions for window resizing operations
 * based on mouse cursor position relative to window borders.
 */
typedef enum ResizeDirection {
    RESIZE_NONE,    /**< No resize operation */
    RESIZE_LEFT,    /**< Resize from left border */
    RESIZE_RIGHT,   /**< Resize from right border */
    RESIZE_TOP,     /**< Resize from top border */
    RESIZE_BOTTOM   /**< Resize from bottom border */
} ResizeDirection;

/*
====================================================================
    TITLEBAR DATA STRUCTURES
====================================================================
*/

/**
 * @brief Title bar component structure
 *
 * Contains all state and properties for window title bar functionality,
 * including window dragging, resizing, maximize/minimize operations.
 */
typedef struct TitleBar {
    Vector2 mousePosition;          /**< Current mouse position */
    Vector2 windowPosition;         /**< Current window position */
    Vector2 panOffset;              /**< Offset for window panning/dragging */
    Vector2 resizeOrigin;           /**< Original position when resize started */

    int screenWidth;                /**< Current screen width */
    int screenHeight;               /**< Current screen height */

    int width;                      /**< Title bar width */
    int height;                     /**< Title bar height */

    bool dragWindow;                /**< Flag indicating if window is being dragged */
    bool exitWindow;                /**< Flag indicating if window should close */
    bool resizing;                  /**< Flag indicating if window is being resized */

    bool isBottonMaximizeClicked;   /**< Flag indicating if maximize button was clicked */
    bool isBottonMinimizeClicked;   /**< Flag indicating if minimize button was clicked */

    ResizeDirection resizeDir;      /**< Current resize direction */

    Context* ctx;                   /**< Reference to application context */
} TitleBar;

/*
====================================================================
    TITLEBAR INITIALIZATION AND UPDATE
====================================================================
*/

/**
 * @brief Creates and initializes title bar component
 *
 * Initializes the TitleBar structure with default values and context reference.
 * Sets up initial dimensions and state flags for window management.
 *
 * @param[out] titleBar TitleBar structure to be initialized
 * @param[in] ctx Application context reference
 *
 * @pre titleBar must be a valid pointer to TitleBar structure
 * @pre ctx must be a valid initialized Context structure
 * @post TitleBar is initialized with height 23 pixels
 * @post All Vector2 fields are set to (0,0)
 * @post All boolean flags are set to false
 * @post resizeDir is set to RESIZE_NONE
 * @post screenWidth/screenHeight are obtained from context
 * @post ctx reference is stored
 *
 * @note Default title bar height is 23 pixels
 * @warning Ensure ctx remains valid for the lifetime of the title bar
 *
 * @see updateTitleBar() for title bar state updates
 *
 * @author Farras
 */
void createTitleBar(TitleBar* titleBar, Context* ctx);

/**
 * @brief Updates title bar component state and handles window interactions
 *
 * Updates the title bar state and processes window drag, resize, maximize,
 * and minimize operations based on mouse interactions.
 *
 * @param[in,out] titleBar TitleBar structure to be updated
 * @param[in] ctx Application context reference
 *
 * @pre titleBar must be a valid initialized TitleBar structure
 * @pre ctx must be a valid Context structure
 * @pre Mouse input must be available for detection
 * @post mousePosition is updated from GetMousePosition()
 * @post Resize direction is detected using GetResizeDirection()
 * @post Cursor is updated with UpdateResizeCursor()
 * @post Window dragging/resizing is handled with SetWindowPosition()/SetWindowSize()
 * @post Maximize/minimize buttons are processed with MaximizeWindow()/MinimizeWindow()/RestoreWindow()
 * @post screenWidth/screenHeight are updated from GetScreenWidth()/GetScreenHeight()
 *
 * @note Handles window dragging when mouse is pressed on title bar
 * @note Supports window resizing from all four borders
 * @note Processes maximize/minimize button interactions
 *
 * @see createTitleBar() for title bar initialization
 * @see drawTitleBar() for title bar rendering
 *
 * @author Farras
 */
void updateTitleBar(TitleBar* titleBar, Context* ctx);

/*
====================================================================
    RENDERING AND DRAWING
====================================================================
*/

/**
 * @brief Renders the title bar component
 *
 * Draws the window title bar with maximize/minimize buttons and resize borders.
 * Provides visual feedback for window management operations.
 *
 * @param[in] titleBar TitleBar structure with valid screenWidth and screenHeight
 *
 * @pre titleBar must be a valid TitleBar structure
 * @pre screenWidth and screenHeight must be properly set
 * @post GuiWindowBox is drawn with title "#198# PORTABLE WINDOW"
 * @post Maximize button (#198#) and minimize button (#35#) are drawn with GuiButton
 * @post Resize borders are drawn on all four window sides with DrawRectangle and GRAY fade color
 * @post Button style border width is configured
 *
 * @note Uses GuiWindowBox for consistent window styling
 * @note Maximize and minimize buttons use icon identifiers #198# and #35#
 * @note Resize borders provide visual indication of resize areas
 * @note Window title displays "#198# PORTABLE WINDOW"
 *
 * @see updateTitleBar() for title bar state management
 *
 * @author Farras
 */
void drawTitleBar(TitleBar* titleBar);

#endif // TITLEBAR_H