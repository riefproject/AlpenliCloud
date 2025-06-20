#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "component.h"
#include "file_manager.h"
#include "item.h"
#include "raylib.h"
#include "sidebar.h"
#include <stdbool.h>

/**
 * @file toolbar.h
 * @brief Toolbar component for file operations and item management in file manager GUI
 * @author AlpenliCloud Development Team
 * @date 2025
 */

#define MAX_FILEPATH_COUNT 16  /**< Maximum number of file paths that can be handled */

 /*
 ====================================================================
     FORWARD DECLARATIONS AND DATA STRUCTURES
 ====================================================================
 */

typedef struct Context Context;

/**
 * @brief Toolbar component structure for file operations
 *
 * Contains all state and properties for the toolbar component,
 * including button states, modal dialogs, and input buffers for
 * file management operations.
 */
typedef struct Toolbar {
    Rectangle currentZeroPosition;  /**< Current layout position and dimensions */

    // Button state flags
    bool isButtonCopyClicked;           /**< Flag indicating if copy button was clicked */
    bool isButtonCutClicked;            /**< Flag indicating if cut button was clicked */
    bool isButtonDeleteClicked;         /**< Flag indicating if delete button was clicked */
    bool isButtonPasteClicked;          /**< Flag indicating if paste button was clicked */
    bool isbuttonRenameClicked;         /**< Flag indicating if rename button was clicked */
    bool isButtonRestoreClicked;        /**< Flag indicating if restore button was clicked */
    bool isButtonPermanentDeleteClicked; /**< Flag indicating if permanent delete button was clicked */
    bool isButtonImportClicked;         /**< Flag indicating if import button was clicked */
    bool isButtonCreateItemClicked;     /**< Flag indicating if create item button was clicked */

    // Import modal properties
    bool importClicked;        /**< Flag indicating if import operation was initiated */
    bool showImportModal;      /**< Flag controlling import modal visibility */
    bool importModalResult;    /**< Result of import modal operation */
    char importPath[512];      /**< Buffer for import file path */

    // Create item modal properties
    bool showCreateModal;         /**< Flag controlling create item modal visibility */
    bool createItemModalResult;   /**< Result of create item modal operation */
    bool dropdownActive;          /**< Flag indicating if dropdown menu is active */
    bool inputCreateItemEditMode; /**< Flag indicating if create item input is in edit mode */
    char inputCreateItemBuffer[MAX_STRING_LENGTH]; /**< Buffer for new item name input */
    ItemType selectedType;        /**< Selected item type (file or folder) */

    // Rename item modal properties
    bool showRenameModal;         /**< Flag controlling rename modal visibility */
    bool renameModalResult;       /**< Result of rename modal operation */
    char renameInputBuffer[MAX_STRING_LENGTH]; /**< Buffer for rename input */

    Context* ctx;                 /**< Reference to application context */
} Toolbar;

/*
====================================================================
    TOOLBAR INITIALIZATION AND UPDATE
====================================================================
*/

/**
 * @brief Creates and initializes toolbar component
 *
 * Initializes the Toolbar structure with default values and configures
 * button properties. Sets up initial state for all modal dialogs and buttons.
 *
 * @param[out] toolbar Toolbar structure to be initialized
 * @param[in] ctx Application context reference
 *
 * @pre toolbar must be a valid pointer to Toolbar structure
 * @pre ctx must be a valid initialized Context structure
 * @post Toolbar is initialized with all button flags set to false
 * @post Modal visibility flags are set to false
 * @post Input buffers are cleared
 * @post selectedType is set to ITEM_FILE
 * @post currentZeroPosition is initialized to empty
 * @post ctx reference is stored
 *
 * @note Default item type for creation is ITEM_FILE
 * @note All input buffers are initialized to empty strings
 * @warning Ensure ctx remains valid for the lifetime of the toolbar
 *
 * @see updateToolbar() for toolbar state updates
 *
 * @author Farras
 */
void createToolbar(Toolbar* toolbar, Context* ctx);

/**
 * @brief Updates toolbar component state and handles file operations
 *
 * Updates toolbar position and processes active button actions with file operations.
 * Handles create, copy, cut, paste, delete, rename, and import operations.
 *
 * @param[in,out] toolbar Toolbar structure to be updated
 * @param[in] ctx Application context reference
 *
 * @pre toolbar must be a valid initialized Toolbar structure
 * @pre ctx must be a valid Context structure with accessible file manager
 * @post currentZeroPosition is updated with y offset and proper positioning
 * @post Button rectangles are repositioned based on current layout
 * @post Modal rectangles are centered on screen
 * @post Create item operations are processed with createFile function
 * @post Copy/cut/delete/paste operations are processed with appropriate file operations
 * @post All button flags are reset after processing
 *
 * @note Processes file operations based on button click states
 * @note Modal dialogs are automatically positioned in screen center
 * @note Button states are reset after each operation
 *
 * @see createToolbar() for toolbar initialization
 * @see drawToolbar() for toolbar rendering
 *
 * @author Farras
 */
void updateToolbar(Toolbar* toolbar, Context* ctx);

/*
====================================================================
    RENDERING AND DRAWING
====================================================================
*/

/**
 * @brief Renders the toolbar component with all buttons
 *
 * Draws toolbar with New, Rename, Cut, Copy, Paste, Delete buttons
 * and separator lines. Layout is responsive based on available width.
 *
 * @param[in] toolbar Toolbar structure with valid currentZeroPosition
 *
 * @pre toolbar must be a valid Toolbar structure
 * @pre currentZeroPosition must be properly set
 * @post New button is drawn with dropdown functionality
 * @post Action buttons (Rename #22#, Cut #17#, Copy #16#, Paste #18#) are drawn on left
 * @post Delete button (#143#) is drawn on right side
 * @post Separator line is drawn at bottom using GuiLine
 * @post Layout adapts to currentZeroPosition width
 *
 * @note Uses icon identifiers for consistent button styling
 * @note Responsive layout adjusts button spacing based on available width
 * @note Separator provides visual distinction from content area
 *
 * @see updateToolbar() for toolbar state management
 * @see DrawNewButtonDropdown() for dropdown rendering
 *
 * @author Farras
 */
void drawToolbar(Toolbar* toolbar);

/**
 * @brief Renders dropdown menu for New button
 *
 * Draws dropdown menu below New button with File/Folder creation options.
 * Handles click detection for option selection and outside click dismissal.
 *
 * @param[in,out] ctx Application context for state management
 * @param[in] btnRect Rectangle defining button position for dropdown placement
 *
 * @pre ctx must be a valid Context structure
 * @pre btnRect must be a valid Rectangle with proper positioning
 * @post Dropdown is drawn below New button with background and border
 * @post File/Folder options are rendered as clickable buttons
 * @post Clicking option activates create item modal for selected type
 * @post Clicking outside dropdown area closes the dropdown
 * @post selectedType is updated based on user selection
 *
 * @note Dropdown automatically positions below the provided button rectangle
 * @note Supports both file and folder creation options
 * @note Outside click detection for intuitive user interaction
 *
 * @see drawToolbar() for main toolbar rendering
 * @see DrawNewItemModal() for item creation modal
 *
 * @author Farras
 */
void DrawNewButtonDropdown(Context* ctx, Rectangle btnRect);

/**
 * @brief Renders modal dialog for creating new items
 *
 * Draws modal dialog for creating new file or folder with name input
 * and Create/Cancel buttons. Handles input validation and modal dismissal.
 *
 * @param[in,out] ctx Application context for modal state management
 *
 * @pre ctx must be a valid Context structure
 * @pre showCreateModal flag must be true for modal visibility
 * @post Modal is displayed centered on screen with overlay background
 * @post Input textbox is rendered for item name entry
 * @post Create button executes item creation if name is valid
 * @post Cancel button or ESC key closes modal without action
 * @post inputCreateItemBuffer is reset after successful creation
 * @post Modal state flags are updated based on user interaction
 *
 * @note Modal includes semi-transparent overlay for focus indication
 * @note Input validation prevents creation of items with invalid names
 * @note Supports keyboard shortcuts (ESC for cancel, Enter for confirm)
 *
 * @see DrawNewButtonDropdown() for triggering item creation
 * @see updateToolbar() for processing creation results
 *
 * @author Farras
 */
void DrawNewItemModal(Context* ctx);

/**
 * @brief Renders modal dialog for renaming selected items
 *
 * Draws modal dialog for renaming selected item with new name input
 * and Rename/Cancel buttons. Validates input and processes rename operation.
 *
 * @param[in,out] ctx Application context for modal state management
 *
 * @pre ctx must be a valid Context structure
 * @pre showRenameModal flag must be true for modal visibility
 * @pre At least one item must be selected for renaming
 * @post Modal is displayed centered on screen with overlay background
 * @post Input textbox is rendered with current item name
 * @post Rename button executes rename operation if new name is valid
 * @post Cancel button or ESC key closes modal without changes
 * @post renameInputBuffer is reset after successful rename
 * @post Modal state flags are updated based on user interaction
 *
 * @note Pre-fills input field with current item name for convenience
 * @note Validates new name to prevent conflicts and invalid characters
 * @note Provides immediate feedback for rename operation results
 *
 * @see drawToolbar() for rename button activation
 * @see updateToolbar() for processing rename results
 *
 * @author Farras
 */
void DrawRenameItemModal(Context* ctx);

/**
 * @brief Renders modal dialog for importing files and folders
 *
 * Draws modal dialog for importing external files/folders with path input
 * and Import/Cancel buttons. Supports path validation and import processing.
 *
 * @param[in,out] ctx Application context for modal state management
 *
 * @pre ctx must be a valid Context structure
 * @pre showImportModal flag must be true for modal visibility
 * @post Modal is displayed centered on screen with overlay background
 * @post Input textbox is rendered for import path entry
 * @post Import button executes import operation if path is valid
 * @post Cancel button or ESC key closes modal without action
 * @post importPath buffer is processed for file system operations
 * @post Modal state flags are updated based on user interaction
 *
 * @note Supports both file and directory import operations
 * @note Path validation ensures import source exists and is accessible
 * @note Provides progress feedback for large import operations
 *
 * @see drawToolbar() for import button activation
 * @see updateToolbar() for processing import results
 *
 * @author GitHub Copilot
 */
void DrawImportModal(Context* ctx);

#endif // TOOLBAR_H