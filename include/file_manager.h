#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "item.h"
#include "nbtree.h"
#include "queue.h"
#include "stack.h"
#include <stdbool.h>
#include "operation.h"

#define alloc(T) (T *)malloc(sizeof(T))

typedef struct Context Context;

/**
 * @brief Main structure for managing file system in the application
 *
 * Contains directory tree, trash, undo/redo operations, and current path.
 * This structure serves as the central management system for all file operations
 * including navigation, CRUD operations, clipboard operations, and search functionality.
 *
 * @author Team AlpenliCloud
 * @version 1.0
 * @since 2025-06-20
 */

typedef struct FileManager {
    Tree root;                ///< Root directory tree
    LinkedList trash;         ///< Root trash container for deleted files
    bool isRootTrash;         ///< Flag indicating if trash is the root trash

    LinkedList searchingList; ///< Linked list storing search results
    bool isSearching;         ///< Flag indicating if currently in search mode

    char* currentPath;        ///< Current path string representation
    Tree treeCursor;          ///< Current tree cursor position

    Stack undo;               ///< Stack for undo operations
    Stack redo;               ///< Stack for redo operations
    bool isCopy;
    Queue copied;             ///< Queue for copied items
    Queue clipboard;          ///< Temporary queue for operations
    LinkedList selectedItem;  ///< Linked list for selected items

    bool needsRefresh;        ///< Flag indicating if refresh is needed
    Context* ctx;             ///< Context for accessing sidebar and other components
} FileManager;

/*
================================================================================
    INITIALIZATION AND SETUP
================================================================================
*/

/**
 * @brief Creates a new file manager instance
 *
 * Initializes all data structures within FileManager with empty/NULL values.
 * This function must be called before any other file manager operations.
 *
 * @param[out] fileManager Pointer to FileManager structure to initialize
 *
 * @pre fileManager must point to allocated memory
 * @post All data structures (tree, stack, queue, list) are created and set to empty/NULL
 *
 * @author Maulana
 */
void createFileManager(FileManager* fileManager);

/**
 * @brief Initializes file manager with filesystem data
 *
 * Loads root directory from filesystem and sets initial treeCursor position.
 * This function populates the file manager with actual filesystem data.
 *
 * @param[in,out] fileManager Pointer to created FileManager instance
 *
 * @pre FileManager must be created but not yet loaded with filesystem data
 * @post Root directory loaded from ROOT path, treeCursor points to root, currentPath set to "root"
 *
 * @author Farras
 */
void initFileManager(FileManager* fileManager);

/**
 * @brief Loads tree structure from filesystem
 *
 * Recursively reads directories and files from filesystem to build tree structure.
 * This function performs deep scanning of directory hierarchy.
 *
 * @param[in,out] tree Tree structure to populate
 * @param[in] path Directory path to scan
 *
 * @pre tree and path are valid
 * @post tree contains file and folder structure matching directory contents recursively
 *
 * @author Farras
 */
void loadTree(Tree tree, char* path);

/**
 * @brief Loads trash data from file
 *
 * Reads trash_dump.txt file to load deleted items into LinkedList trash.
 * This function restores trash state from persistent storage.
 *
 * @param[out] trash Pointer to LinkedList to populate with trash items
 *
 * @pre trash_dump.txt file exists and contains valid data
 * @post LinkedList trash populated with deleted items, each containing name, original path, deletion time, and trash path
 *
 * @author Farras
 */
void loadTrashFromFile(LinkedList* trash);

/**
 * @brief Saves trash data to file
 *
 * Persists all items in LinkedList trash to trash_dump.txt file.
 * This function ensures trash state survives application restarts.
 *
 * @param[in] fileManager Pointer to FileManager containing trash data
 *
 * @pre LinkedList trash contains deleted items
 * @post trash_dump.txt file updated with trash data, each item saved in "name,originalPath,deletedTime" format
 *
 * @author Farras
 */
void saveTrashToFile(FileManager* fileManager);

/**
 * @brief Prints trash contents to console
 *
 * Displays all items in LinkedList trash to console for debugging/monitoring.
 *
 * @param[in] trash LinkedList containing trash items
 *
 * @pre LinkedList trash contains deleted items
 * @post Each item printed to console with name, original path, deletion time, and trash path
 *
 * @author Farras
 */
void printTrash(LinkedList trash);

/**
 * @brief Refreshes file manager with current filesystem state
 *
 * Reloads tree structure from current directory by clearing children and reloading.
 * This function synchronizes in-memory tree with actual filesystem state.
 *
 * @param[in,out] fileManager Pointer to FileManager to refresh
 *
 * @pre Tree structure may be out of sync with filesystem
 * @post Child nodes cleared, tree reloaded from treeCursor path, tree structure synchronized with filesystem
 *
 * @author Arief
 * @editor Farras
 */
void refreshFileManager(FileManager* fileManager);

/*
================================================================================
    FILE AND FOLDER OPERATIONS
================================================================================
*/

/**
 * @brief Searches for a file by path
 *
 * Finds a file based on path by creating dummy item and using searchTree.
 * Returns the found item or empty item if not found.
 *
 * @param[in] fileManager Pointer to FileManager instance
 * @param[in] path File path to search for
 * @return Item structure containing file information, or empty item with all fields 0/NULL if not found
 *
 * @pre path is valid and non-NULL
 * @post File item returned if found, empty item otherwise
 *
 * @author Maulana
 */
Item searchFile(FileManager* fileManager, char* path);

/**
 * @brief Searches for all files/folders matching keyword
 *
 * Searches all files/folders matching keyword in current directory and populates searchingList.
 * This function enables content discovery within the current directory context.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 * @param[in] keyword Search keyword string
 *
 * @pre keyword is valid search string
 * @post LinkedList searchingList populated with matching items, each containing name, path, size, type, and time
 *
 * @author Farras
 */
void searchingTreeItem(FileManager* fileManager, char* keyword);

/**
 * @brief Recursively searches items in tree
 *
 * Recursively searches all items matching keyword in tree structure.
 * This function provides deep search capability across directory hierarchy.
 *
 * @param[out] linkedList LinkedList to store search results
 * @param[in] tree Tree to search in
 * @param[in] keyword Search keyword string
 *
 * @pre linkedList, tree, and keyword are valid
 * @post LinkedList populated with matching items, each containing name, path, size, type, and time
 *
 * @author Farras
 */
void searchingTreeItemRecursive(LinkedList* linkedList, Tree tree, char* keyword);

/**
 * @brief Searches items in linked list
 *
 * Searches items in LinkedList based on keyword by comparing item names.
 * This function enables search within list-based data structures.
 *
 * @param[in,out] FileManager Pointer to FileManager instance
 * @param[in] node Node to search from
 * @param[in] keyword Search keyword string
 *
 * @pre FileManager, node, and keyword are valid
 * @post LinkedList populated with matching items, each containing name, path, size, type, and time
 *
 * @author Farras
 */
void searchingLinkedListItem(FileManager* FileManager, Node* node, char* keyword);

/**
 * @brief Recursively searches items in linked list
 *
 * Recursively searches items in LinkedList based on keyword.
 * This function provides comprehensive search within linked list structures.
 *
 * @param[in,out] FileManager Pointer to FileManager instance
 * @param[in] node Node to search from
 * @param[in] keyword Search keyword string
 *
 * @pre FileManager, node, and keyword are valid
 * @post LinkedList populated with matching items, each containing name, path, size, type, and time
 *
 * @author Farras
 */
void searchingLinkedListRecursive(FileManager* FileManager, Node* node, char* keyword);

/**
 * @brief Prints search results to console
 *
 * Displays all items in LinkedList searchingList to console.
 * This function provides visual feedback for search operations.
 *
 * @param[in] fileManager Pointer to FileManager containing search results
 *
 * @pre LinkedList searchingList contains found items
 * @post Each item printed to console with name, path, size, type, and time
 *
 * @author Farras
 */
void printSearchingList(FileManager* fileManager);

/**
 * @brief Creates a new file or folder
 *
 * Creates new file or folder in directory with duplication checking and undo operation support.
 * This function handles both file and folder creation with conflict resolution.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 * @param[in] type Type of item to create (file or folder)
 * @param[in] dirPath Directory path where item will be created
 * @param[in] name Name of new item
 * @param[in] isOperation Flag indicating if operation should be saved for undo
 *
 * @pre type, dirPath, and name are valid
 * @post File/folder created in filesystem, added to tree, operation saved for undo if isOperation=true
 *
 * @author Maulana
 */
void createFile(FileManager* fileManager, ItemType type, char* dirPath, char* name, bool isOperation);

/**
 * @brief Deletes files/folders to trash
 *
 * Moves all selected files/folders to physical trash directory and LinkedList trash.
 * This function provides safe deletion with recovery capability.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 * @param[in] isOperation Flag indicating if operation should be saved for undo
 *
 * @pre selectedItem contains files/folders selected for deletion
 * @post Files/folders moved to trash directory, added to LinkedList trash, removed from main tree, operation saved for undo
 *
 * @author Arief
 */
void deleteFile(FileManager* fileManager, bool isOperation);

/**
 * @brief Permanently deletes files/folders
 *
 * Permanently deletes files/folders from filesystem without saving to trash.
 * This function provides irreversible deletion capability.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 *
 * @pre selectedItem contains files/folders selected for permanent deletion
 * @post Files/folders permanently deleted from filesystem, removed from main tree, operation saved for undo if isOperation=true
 *
 * @author Farras
 */
void deletePermanentFile(FileManager* fileManager);

/**
 * @brief Renames a file or folder
 *
 * Changes name of file or folder with duplication checking and filesystem operations.
 * This function handles name conflicts and maintains data integrity.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 * @param[in] filePath Path of file to rename
 * @param[in] newName New name for the file
 * @param[in] isOperation Flag indicating if operation should be saved for undo
 *
 * @pre filePath and newName are valid
 * @post File/folder renamed in filesystem, item in tree updated with new name and path, operation saved for undo if isOperation=true
 *
 * @author Maulana
 */
void renameFile(FileManager* fileManager, char* filePath, char* newName, bool isOperation);

/**
 * @brief Recovers files from trash
 *
 * Restores selected files/folders from LinkedList trash to main tree and filesystem.
 * This function enables recovery of accidentally deleted items.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 *
 * @pre selectedItem contains files/folders that exist in LinkedList trash
 * @post Files/folders moved from trash to recovery location, removed from LinkedList trash, added back to tree
 *
 * @author Arief
 */
void recoverFile(FileManager* fileManager);

/*
================================================================================
    COPY, CUT, PASTE OPERATIONS
================================================================================
*/

/**
 * @brief Copies selected files/folders
 *
 * Copies all selected files/folders to copied queue and sets copy mode.
 * This function enables clipboard-style copy operations.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 *
 * @pre selectedItem contains files/folders to copy
 * @post Files/folders copied to copied queue, isCopy set to true, temp queue contains copied data
 *
 * @author Arief
 */
void copyFile(FileManager* fileManager);

/**
 * @brief Cuts selected files/folders
 *
 * Cuts all selected files/folders to copied queue and sets cut mode.
 * This function enables clipboard-style cut operations.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 *
 * @pre selectedItem contains files/folders to cut
 * @post Files/folders copied to copied queue, isCopy set to false, temp queue contains copied data
 *
 * @author Arief
 */
void cutFile(FileManager* fileManager);

/**
 * @brief Pastes files/folders from clipboard
 *
 * Pastes files/folders from temp queue to current directory with progress bar for large operations.
 * This function completes clipboard operations with visual feedback for bulk operations.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 * @param[in] isOperation Flag indicating if operation should be saved for undo
 *
 * @pre temp queue contains files/folders from copy/cut operation
 * @post Files/folders pasted to treeCursor with copy content or move, progress bar shown if >10 items, temp queue cleared for cut
 *
 * @author Arief
 */
void pasteFile(FileManager* fileManager, bool isOperation);

/*
================================================================================
    FILE SELECTION OPERATIONS
================================================================================
*/

/**
 * @brief Selects a file/folder
 *
 * Adds item to LinkedList selectedItem if not already present.
 * This function manages item selection state.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 * @param[in,out] item Pointer to item to select
 *
 * @pre item is valid
 * @post item added to selectedItem if not already present, item.selected set to true
 */
void selectFile(FileManager* fileManager, Item* item);

/**
 * @brief Deselects a file/folder
 *
 * Removes item from LinkedList selectedItem and sets selected=false.
 * This function manages item deselection state.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 * @param[in,out] item Pointer to item to deselect
 *
 * @pre item may exist in selectedItem
 * @post item removed from selectedItem if found, item.selected set to false, LinkedList reset if empty
 */
void deselectFile(FileManager* fileManager, Item* item);

/**
 * @brief Clears all selected files
 *
 * Removes all items from LinkedList selectedItem by calling deselectFile for each item.
 * This function provides bulk deselection capability.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 *
 * @pre selectedItem contains selected items
 * @post All items deselected, selectedItem becomes empty
 */
void clearSelectedFile(FileManager* fileManager);

/**
 * @brief Selects all files in current directory
 *
 * Selects all child nodes from current treeCursor.
 * This function provides bulk selection capability.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 *
 * @pre treeCursor points to valid directory with child nodes
 * @post All child nodes from treeCursor selected and added to selectedItem
 */
void selectAll(FileManager* fileManager);

/*
================================================================================
    UNDO AND REDO OPERATIONS
================================================================================
*/

/**
 * @brief Undoes the last operation
 *
 * Reverts last operation from undo stack according to operation type.
 * This function provides operation rollback capability.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 *
 * @pre undo stack contains operations
 * @post Operation reverted according to type (CREATE→delete, DELETE→restore, UPDATE→rename back), operation moved to redo stack, filesystem refreshed
 *
 * @author Maulana
 */
void undo(FileManager* fileManager);

/**
 * @brief Redoes previously undone operation
 *
 * Repeats operation that was previously undone from redo stack.
 * This function provides operation replay capability.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 *
 * @pre redo stack contains operations
 * @post Operation executed again according to type, operation moved back to undo stack
 *
 * @author Maulana
 */
void redo(FileManager* fileManager);

/*
================================================================================
    DIRECTORY NAVIGATION
================================================================================
*/

/**
 * @brief Gets current root directory
 *
 * Finds and returns root directory from treeCursor by traversing to parent.
 * This function provides root directory access.
 *
 * @param[in] fileManager FileManager instance
 * @return Tree pointer to root directory, or NULL if invalid
 *
 * @pre FileManager and treeCursor are valid
 * @post Root directory found and returned, or NULL if invalid
 *
 * @author Farras
 */
Tree getCurrentRoot(FileManager fileManager);

/**
 * @brief Extracts filename from complete file path
 *
 * Returns pointer to filename portion of path by finding last slash.
 * Handles both forward and backward slash separators.
 *
 * @param[in] path Complete file path
 * @return Pointer to filename portion of path
 *
 * @pre path is valid string
 * @post Filename portion returned or entire path if no separator found
 *
 * @internal
 * @since 1.0
 */
char* getNameFromPath(char* path);

/**
 * @brief Navigates back to parent directory
 *
 * Moves treeCursor to parent directory if available.
 * This function provides backward navigation capability.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 *
 * @pre treeCursor not at root directory
 * @post treeCursor moved to parent directory, currentPath updated, filesystem refreshed
 *
 * @author Farras
 */
void goBack(FileManager* fileManager);

/**
 * @brief Navigates to specific directory
 *
 * Moves treeCursor to target tree and updates currentPath.
 * This function provides direct navigation capability.
 *
 * @param[in,out] FileManager Pointer to FileManager instance
 * @param[in] tree Target tree to navigate to
 *
 * @pre tree target is valid and accessible
 * @post treeCursor moved to tree target, currentPath updated with complete path, filesystem refreshed
 *
 * @author Farras
 */
void goTo(FileManager* FileManager, Tree tree);

/**
 * @brief Sorts child nodes by type
 *
 * Sorts child nodes by type using insertion sort (folders first, then files).
 * This function provides organized directory display.
 *
 * @param[in,out] parent Pointer to parent node to sort children
 *
 * @pre parent node has child nodes that may be unsorted
 * @post child nodes sorted by ItemType (ITEM_FOLDER < ITEM_FILE)
 *
 * @author Farras
 */
void sort_children(Tree* parent);

/*
================================================================================
    UTILITY AND HELPER FUNCTIONS
================================================================================
*/

/**
 * @brief Opens file with Windows "Open With" dialog
 *
 * Opens Windows "Open With" dialog for specific file using command line.
 * This function provides system integration for file opening.
 *
 * @param[in] path Path to file to open
 *
 * @pre path is valid and accessible
 * @post Command "cmd /c start path /OPENAS" executed, Windows "Open With" dialog opened
 *
 * @author Farras
 */
void windowsOpenWith(char* path);

/*
================================================================================
    IMPORT/UPLOAD FILE OPERATIONS
================================================================================
*/

/**
 * @brief Imports file/folder from external path
 *
 * Imports file or folder from path outside workspace to current directory.
 * This function enables external content integration.
 *
 * @param[in,out] fileManager Pointer to FileManager instance
 * @param[in] sourcePath Source path to import from
 * @param[in] isOperation Flag indicating if operation should be saved for undo
 *
 * @pre sourcePath is valid and accessible, destination is current treeCursor
 * @post File/folder copied to current directory with duplicate name handling, item added to tree structure, operation saved for undo
 *
 * @author GitHub Copilot
 */
void importFile(FileManager* fileManager, char* sourcePath, bool isOperation);

#endif // !FILE_MANAGER_H