#ifndef OPERATION_H
#define OPERATION_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "queue.h"

/**
 * @file operation.h
 * @brief Operation management for file system actions with undo/redo support
 * @author AlpenliCloud Development Team
 * @date 2025
 */

 /*
 ====================================================================
     ENUMERATIONS AND DATA STRUCTURES
 ====================================================================
 */

 /**
  * @brief Enumeration of supported file operation types
  *
  * Defines the types of file system operations that can be performed
  * and tracked for undo/redo functionality.
  */
typedef enum ActionType {
    ACTION_CREATE,  /**< Create new file or directory */
    ACTION_DELETE,  /**< Delete existing file or directory */
    ACTION_UPDATE,  /**< Update/modify existing file */
    ACTION_RECOVER, /**< Recover deleted file or directory */
    ACTION_PASTE    /**< Paste file or directory (copy/move) */
} ActionType;

/**
 * @brief Structure representing a file system operation
 *
 * Contains all necessary information to perform and potentially undo
 * a file system operation. Used for implementing undo/redo functionality.
 */
typedef struct Operation {
    char* from;        /**< Source path of the operation */
    char* to;          /**< Destination path of the operation */
    ActionType type;   /**< Type of operation being performed */
    bool isDir;        /**< Flag indicating if operation is on a directory */
    bool isCopy;       /**< Flag indicating if operation is a copy (vs move) */
    Queue* itemTemp;   /**< Temporary queue for storing operation items */
} Operation;

/*
====================================================================
    OPERATION CONSTRUCTORS
====================================================================
*/

/**
 * @brief Creates a new file system operation
 *
 * Constructs a new Operation structure with the specified parameters.
 * The from and to paths are duplicated using strdup for memory safety.
 *
 * @param[in] from Source path for the operation
 * @param[in] to Destination path for the operation
 * @param[in] type Type of operation to perform
 * @param[in] isDir Flag indicating if this is a directory operation
 * @param[in] itemTemp Temporary queue for operation items (can be NULL)
 *
 * @return Operation Newly created operation structure
 *
 * @pre from and to must be valid null-terminated strings
 * @pre type must be a valid ActionType value
 * @post New Operation is created with duplicated path strings
 * @post isCopy flag is initialized to false by default
 *
 * @note The from and to strings are duplicated using strdup()
 * @note Caller is responsible for freeing the Operation when done
 * @warning Ensure proper memory management for the returned Operation
 *
 * @see ActionType for available operation types
 *
 * @author Maulana
 */
Operation createOperation(char* from, char* to, ActionType type, bool isDir, Queue* itemTemp);

#endif // OPERATION_H