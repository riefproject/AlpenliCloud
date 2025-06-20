#ifndef ITEM_H
#define ITEM_H

#include <stdio.h>
#include <time.h>
#include "queue.h"
#include "linked.h"

/**
 * @file item.h
 * @brief File system item structures and operations for file manager
 * @author AlpenliCloud Development Team
 * @date 2025
 */

 /*
 ====================================================================
     ENUMERATIONS
 ====================================================================
 */

 /**
  * @brief Enumeration of file system item types
  *
  * Defines the different types of items that can exist in the file system.
  */
typedef enum ItemType {
  ITEM_FOLDER,  /**< Directory/folder item */
  ITEM_FILE     /**< Regular file item */
} ItemType;

/*
====================================================================
    DATA STRUCTURES
====================================================================
*/

/**
 * @brief Structure representing a file system item
 *
 * Contains all metadata and information about a file or directory
 * in the file system, including timestamps and selection state.
 */
typedef struct Item {
  char* name;           /**< Name of the item (filename or directory name) */
  char* path;           /**< Full path to the item */
  long size;            /**< Size of the item in bytes (0 for directories) */
  ItemType type;        /**< Type of item (file or folder) */
  time_t created_at;    /**< Creation timestamp */
  time_t updated_at;    /**< Last modification timestamp */
  time_t deleted_at;    /**< Deletion timestamp (0 if not deleted) */
  bool selected;        /**< Selection state for UI operations */
} Item;

/**
 * @brief Structure representing an item in the trash/recycle bin
 *
 * Extends the basic Item structure with additional metadata needed
 * for trash management and restoration functionality.
 */
typedef struct TrashItem {
  Item item;            /**< Base item structure */
  char* originalPath;   /**< Original path before deletion */
  time_t deletedTime;   /**< Timestamp when item was deleted */
  char* trashPath;      /**< Path within the physical trash folder */
  char* uid;            /**< Unique identifier for the trash item */
} TrashItem;

/**
 * @brief Structure representing an item prepared for paste operation
 *
 * Contains item information along with its original location for
 * copy/cut and paste operations.
 */
typedef struct PasteItem {
  Item item;            /**< Base item structure */
  char* originalPath;   /**< Original path before cut/copy operation */
} PasteItem;

/*
====================================================================
    ITEM CONSTRUCTORS
====================================================================
*/

/**
 * @brief Creates a new file system item
 *
 * Constructs a new Item structure with all fields populated and selection
 * state set to false. String fields are duplicated using strdup for memory safety.
 *
 * @param[in] name Name of the item
 * @param[in] path Full path to the item
 * @param[in] size Size of the item in bytes
 * @param[in] type Type of item (file or folder)
 * @param[in] created_at Creation timestamp
 * @param[in] updated_at Last modification timestamp
 * @param[in] deleted_at Deletion timestamp
 *
 * @return Item Newly created item structure
 *
 * @pre name and path must be valid null-terminated strings
 * @pre size must be non-negative
 * @pre type must be a valid ItemType value
 * @post New Item is created with duplicated string fields
 * @post selected field is initialized to false
 *
 * @note String fields are duplicated using strdup()
 * @note Caller is responsible for freeing the Item when done
 * @warning Ensure proper memory management for string fields
 *
 * @see ItemType for available item types
 *
 * @author Maulana
 */
Item createItem(char* name, char* path, long size, ItemType type, time_t created_at, time_t updated_at, time_t deleted_at);

/**
 * @brief Creates a new paste item
 *
 * Constructs a new PasteItem structure with the given item and original path.
 * The original path is duplicated using strdup for memory safety.
 *
 * @param[in] item Base item structure to be included
 * @param[in] originalPath Original path before cut/copy operation
 *
 * @return PasteItem Newly created paste item structure
 *
 * @pre item must be a valid Item structure
 * @pre originalPath must be a valid null-terminated string
 * @post New PasteItem is created with duplicated originalPath
 *
 * @note originalPath is duplicated using strdup()
 * @note Caller is responsible for freeing the PasteItem when done
 * @warning Ensure proper memory management for originalPath field
 *
 * @see Item for base item structure
 *
 * @author Maulana
 */
PasteItem createPasteItem(Item item, char* originalPath);

/*
====================================================================
    TRASH ITEM OPERATIONS
====================================================================
*/

/**
 * @brief Searches for a trash item by original path
 *
 * Searches through the trash LinkedList to find a TrashItem with matching
 * original path using string comparison.
 *
 * @param[in] trash LinkedList containing TrashItem structures
 * @param[in] originalPath Original path to search for
 *
 * @return TrashItem Found trash item or empty TrashItem if not found
 * @retval TrashItem Matching trash item if found
 * @retval Empty TrashItem All fields set to 0/NULL if not found or trash is empty
 *
 * @pre trash must be a valid LinkedList
 * @pre originalPath must be a valid null-terminated string
 * @post No modification to the trash list
 *
 * @note Uses strcmp() for path comparison
 * @note Returns empty TrashItem structure if item not found
 * @warning Check returned TrashItem fields before use
 *
 * @see LinkedList for list structure
 * @see TrashItem for trash item structure
 *
 * @author Maulana
 */
TrashItem searchTrashItem(LinkedList trash, char* originalPath);

#endif // ITEM_H