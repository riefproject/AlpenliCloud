#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "file_manager.h"
#include "raylib.h"

/**
 * @file sidebar.h
 * @brief Sidebar component for hierarchical folder navigation in file manager GUI
 * @author AlpenliCloud Development Team
 * @date 2025
 */

#define BASE_SIDEBAR_PANEL_CONTENT_WIDTH 150  /**< Base width for sidebar panel content */

 /*
 ====================================================================
     FORWARD DECLARATIONS AND DATA STRUCTURES
 ====================================================================
 */

typedef struct Context Context;

/**
 * @brief Sidebar item structure for hierarchical folder representation
 *
 * Represents a single item in the sidebar tree structure with expansion state
 * and hierarchical relationships between folders.
 */
typedef struct SidebarItem SidebarItem;
typedef struct SidebarItem {
    Tree tree;                  /**< Associated tree node from file manager */
    bool isExpanded;           /**< Flag indicating if folder is expanded */
    SidebarItem* first_son;    /**< Pointer to first child item */
    SidebarItem* next_brother; /**< Pointer to next sibling item */
} SidebarItem;

/**
 * @brief Sidebar state structure for preserving expansion states
 *
 * Used to maintain expansion states of sidebar items during rebuilds
 * and updates to preserve user interface state.
 */
typedef struct SidebarState {
    Tree treeNode;              /**< Associated tree node */
    bool isExpanded;           /**< Expansion state to preserve */
    struct SidebarState* next; /**< Next state in linked list */
} SidebarState;

/**
 * @brief Main sidebar component structure
 *
 * Contains all state and layout information for the sidebar component,
 * including scroll position, panel dimensions, and tree structure.
 */
typedef struct Sidebar {
    Rectangle panelRec;        /**< Overall sidebar panel dimensions */
    Rectangle panelContentRec; /**< Content area dimensions within sidebar */
    Rectangle panelView;       /**< Visible area dimensions (excluding scrollbars) */
    Vector2 panelScroll;       /**< Current scroll position */

    SidebarItem* sidebarRoot;  /**< Root of sidebar tree structure */
    Rectangle currentZeroPosition; /**< Current layout position reference */

    bool isButtonOpenTrashClicked; /**< Flag for trash button click state */
    bool isButtonGoBackClicked;    /**< Flag for back button click state */

    Context* ctx;              /**< Reference to application context */
} Sidebar;

/*
====================================================================
    SIDEBAR INITIALIZATION AND UPDATE
====================================================================
*/

/**
 * @brief Creates and initializes sidebar component
 *
 * Initializes the Sidebar structure with default values and context reference.
 * Sets up panel dimensions and scroll position to defaults.
 *
 * @param[out] sidebar Sidebar structure to be initialized
 * @param[in] ctx Application context reference
 *
 * @pre sidebar must be a valid pointer to Sidebar structure
 * @pre ctx must be a valid initialized Context structure
 * @post Sidebar is initialized with empty panelRec
 * @post panelContentRec is set to (0,0,160,340)
 * @post panelView is initialized to empty
 * @post panelScroll is set to (0,0)
 * @post sidebarRoot is set to NULL
 * @post ctx reference is stored
 *
 * @note Initial panel content size is set to 160x340 pixels
 * @warning Ensure ctx remains valid for the lifetime of the sidebar
 *
 * @see updateSidebar() for sidebar state updates
 *
 * @author Farras
 */
void createSidebar(Sidebar* sidebar, Context* ctx);

/**
 * @brief Updates sidebar component dimensions and loads tree structure
 *
 * Updates sidebar size and position, and loads sidebarRoot from fileManager.
 * Rebuilds the sidebar tree structure when necessary.
 *
 * @param[in,out] sidebar Sidebar structure to be updated
 * @param[in] ctx Application context reference
 *
 * @pre sidebar must be a valid initialized Sidebar structure
 * @pre ctx must be a valid Context structure with accessible file manager
 * @post sidebarRoot is created from getCurrentRoot if still NULL
 * @post currentZeroPosition is updated with y offset and height adjustment
 * @post panelRec is calculated with width 170 and height based on currentZeroPosition
 *
 * @note Creates sidebar tree from file manager's current root
 * @note Panel width is fixed at 170 pixels
 *
 * @see createSidebar() for sidebar initialization
 * @see drawSidebar() for sidebar rendering
 *
 * @author Farras
 */
void updateSidebar(Sidebar* sidebar, Context* ctx);

/*
====================================================================
    SIDEBAR ITEM CONSTRUCTORS
====================================================================
*/

/**
 * @brief Creates sidebar item recursively from tree structure
 *
 * Creates SidebarItem recursively from Tree using malloc and initializes
 * expansion state to false. Builds complete hierarchical structure.
 *
 * @param[in] tree Tree node to create sidebar item from (can be NULL)
 *
 * @return SidebarItem* Pointer to created SidebarItem or NULL if tree is NULL
 * @retval SidebarItem* Valid sidebar item with tree structure
 * @retval NULL If input tree is NULL or memory allocation fails
 *
 * @pre tree can be NULL or valid with first_son and next_brother structure
 * @post SidebarItem is created with malloc
 * @post tree is referenced in created item
 * @post isExpanded is set to false
 * @post first_son and next_brother are created recursively
 *
 * @note Memory is allocated using malloc for each item
 * @note Recursively processes entire tree structure
 * @warning Caller is responsible for freeing allocated memory
 *
 * @see createSidebarItemWithState() for creating with preserved state
 * @see destroySidebarItem() for memory cleanup
 *
 * @author Farras
 */
SidebarItem* crateSidebarItem(Tree tree);

/**
 * @brief Creates sidebar item with preserved expansion states
 *
 * Creates SidebarItem from tree while preserving expansion states from
 * the provided state list. Used for rebuilding sidebar while maintaining UI state.
 *
 * @param[in] root Root tree node to create sidebar from
 * @param[in] stateList List of preserved expansion states
 *
 * @return SidebarItem* Pointer to created SidebarItem with preserved states
 *
 * @pre root must be a valid Tree structure
 * @pre stateList can be NULL or valid SidebarState linked list
 * @post SidebarItem is created with expansion states from stateList
 * @post Items not in stateList default to collapsed state
 *
 * @note Preserves user's expanded/collapsed preferences during rebuilds
 * @see crateSidebarItem() for basic item creation
 * @see collectSidebarState() for collecting current states
 *
 * @author Farras
 */
SidebarItem* createSidebarItemWithState(Tree root, SidebarState* stateList);

/**
 * @brief Destroys sidebar item and frees allocated memory
 *
 * Recursively destroys SidebarItem structure and frees all allocated memory.
 * Sets the pointer to NULL after cleanup.
 *
 * @param[in,out] item Pointer to SidebarItem pointer to be destroyed
 *
 * @pre item must be a valid pointer to SidebarItem pointer
 * @post All child items are recursively destroyed
 * @post All allocated memory is freed
 * @post item pointer is set to NULL
 *
 * @note Recursively frees entire tree structure
 * @warning Ensure no references to destroyed items remain
 *
 * @see crateSidebarItem() for item creation
 *
 * @author Farras
 */
void destroySidebarItem(SidebarItem** item);

/**
 * @brief Collects current expansion states from sidebar items
 *
 * Traverses sidebar tree and collects expansion states into a linked list
 * for preservation during sidebar rebuilds.
 *
 * @param[in] item Root sidebar item to collect states from
 * @param[out] stateList Pointer to state list to be populated
 *
 * @pre item can be NULL or valid SidebarItem
 * @pre stateList must be a valid pointer to SidebarState pointer
 * @post stateList contains expansion states of all items
 * @post States are stored in linked list format
 *
 * @note Used before rebuilding sidebar to preserve UI state
 * @see createSidebarItemWithState() for using collected states
 * @see destroySidebarState() for cleanup
 *
 * @author Farras
 */
void collectSidebarState(SidebarItem* item, SidebarState** stateList);

/**
 * @brief Gets expansion state for specific tree from state list
 *
 * Searches state list for expansion state of specific tree node.
 * Returns false if tree not found in state list.
 *
 * @param[in] tree Tree node to find state for
 * @param[in] stateList List of sidebar states to search
 *
 * @return bool Expansion state of the tree
 * @retval true Tree is expanded in state list
 * @retval false Tree is collapsed or not found in state list
 *
 * @pre tree must be a valid Tree structure
 * @pre stateList can be NULL or valid SidebarState list
 *
 * @note Returns false for trees not found in state list
 * @see collectSidebarState() for creating state lists
 *
 * @author Farras
 */
bool getExpandedForTree(Tree tree, SidebarState* stateList);

/**
 * @brief Destroys sidebar state list and frees memory
 *
 * Frees all nodes in the sidebar state linked list and associated memory.
 *
 * @param[in,out] stateList Head of state list to be destroyed
 *
 * @pre stateList can be NULL or valid SidebarState list
 * @post All state nodes are freed
 * @post All associated memory is released
 *
 * @note Safely handles NULL input
 * @see collectSidebarState() for state list creation
 *
 * @author Farras
 */
void destroySidebarState(SidebarState* stateList);

/*
====================================================================
    RENDERING AND DRAWING
====================================================================
*/

/**
 * @brief Renders the sidebar component with scrollable content
 *
 * Draws sidebar with scroll panel and all folder items in tree structure.
 * Handles scrolling and content sizing automatically.
 *
 * @param[in,out] sidebar Sidebar structure with initialized sidebarRoot
 *
 * @pre sidebar must be valid with initialized sidebarRoot
 * @post GuiScrollPanel is drawn with scissor mode enabled
 * @post drawPos is calculated from panelRec with scroll offset
 * @post drawSidebarItem is called to render all items
 * @post panelContentRec width and height are updated based on content
 *
 * @note Uses GuiScrollPanel for automatic scrollbar handling
 * @note Content dimensions are dynamically calculated
 *
 * @see drawSidebarItem() for individual item rendering
 * @see updateSidebar() for sidebar updates
 *
 * @author Farras
 */
void drawSidebar(Sidebar* sidebar);

/**
 * @brief Renders sidebar item recursively with mouse interaction
 *
 * Draws sidebar item recursively with indentation and handles mouse interactions
 * for expansion/collapse and navigation.
 *
 * @param[in,out] sidebar Sidebar structure for state management
 * @param[in,out] node SidebarItem node to draw
 * @param[in] fileManager File manager for navigation operations
 * @param[in,out] pos Current drawing position
 * @param[in] depth Current indentation depth
 * @param[in] width Item width
 * @param[in] height Item height
 * @param[out] scrollWidth Maximum content width for scrolling
 *
 * @pre node must be a valid SidebarItem
 * @pre pos, width, height must be valid values
 * @pre scrollWidth must be a valid pointer
 * @post Folder item is drawn with arrow icon (#115#/#116#)
 * @post Indentation is applied based on depth
 * @post Mouse clicks are handled for expand/collapse and navigation
 * @post Background is BLUE for active treeCursor
 * @post scrollWidth is updated based on content width
 * @post Child items are drawn recursively if expanded
 *
 * @note Uses arrow icons #115# (collapsed) and #116# (expanded)
 * @note Supports mouse interaction for folder operations
 * @note Recursive rendering for tree structure
 *
 * @see drawSidebar() for main sidebar rendering
 * @see getMaxChildLabelWidth() for width calculations
 *
 * @author Farras
 */
void drawSidebarItem(Sidebar* sidebar, SidebarItem* node, FileManager* fileManager, Vector2* pos, int depth, float width, float height, float* scrollWidth);

/*
====================================================================
    UTILITY FUNCTIONS
====================================================================
*/

/**
 * @brief Calculates maximum child label width for layout optimization
 *
 * Computes maximum label width from all child nodes for proper sidebar layout.
 * Used for determining optimal content width and scrolling requirements.
 *
 * @param[in] node SidebarItem node to analyze
 * @param[in] depth Current indentation depth
 * @param[in] textSize Text size for width calculations
 *
 * @return int Maximum label width in pixels
 *
 * @pre node can be NULL or valid SidebarItem
 * @pre depth and textSize must be positive values
 * @post Maximum label width is calculated with recursive traversal
 * @post Returns width including indentation offset
 *
 * @note Recursively traverses all child nodes
 * @note Includes indentation in width calculations
 * @note Used for optimizing sidebar layout and scrolling
 *
 * @see drawSidebarItem() for label rendering
 *
 * @author Farras
 */
int getMaxChildLabelWidth(SidebarItem* node, int depth, int textSize);

#endif // SIDEBAR_H