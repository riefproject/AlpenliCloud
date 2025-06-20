#ifndef WIN_UTILS_H
#define WIN_UTILS_H

/**
 * @file win_utils.h
 * @brief Windows-specific utility functions for file operations and system dialogs
 * @author AlpenliCloud Development Team
 * @date 2025
**/

/*
=====================================================================
    ENUMERATIONS
=====================================================================
*/

/**
   * @brief Enumeration of Windows common folders
   *
   * Defines constants for accessing standard Windows user folders
   * such as Desktop, Documents, Downloads, etc.
   */
typedef enum {
    WIN_FOLDER_DESKTOP,    /**< User's Desktop folder */
    WIN_FOLDER_DOCUMENTS,  /**< User's Documents folder */
    WIN_FOLDER_DOWNLOADS,  /**< User's Downloads folder */
    WIN_FOLDER_PICTURES,   /**< User's Pictures folder */
    WIN_FOLDER_MUSIC,      /**< User's Music folder */
    WIN_FOLDER_VIDEOS      /**< User's Videos folder */
} WindowsCommonFolder;

/*
====================================================================
    FILE OPERATIONS
====================================================================
*/

/**
 * @brief Recursively removes directory and all its contents
 *
 * Performs recursive deletion of a directory including all subdirectories
 * and files contained within. Uses Windows-specific APIs for efficient
 * file system operations.
 *
 * @param[in] folderPath Path to the folder to be removed
 *
 * @return int Status code
 * @retval 0 Success - folder and all contents removed
 * @retval -1 Failure - operation failed (insufficient permissions, folder in use, etc.)
 *
 * @pre folderPath must be a valid directory path
 * @post Directory and all contents are permanently deleted from file system
 *
 * @note This operation is irreversible and does not use Windows Recycle Bin
 * @warning Ensure proper permissions and that no files are in use before calling
 * @warning folderPath parameter must be a valid null-terminated string
 *
 * @see OpenWindowsFolderDialog() for folder selection
 */
int RemoveItemsRecurse(const char* folderPath);

/*
====================================================================
    SYSTEM DIALOGS
====================================================================
*/

/**
 * @brief Opens Windows file selection dialog
 *
 * Displays the standard Windows file open dialog allowing user to select
 * a single file. The selected file path is returned in the provided buffer.
 *
 * @param[out] filePath Buffer to store the selected file path
 * @param[in] maxPathLength Maximum length of the file path buffer
 *
 * @return int Status code
 * @retval 1 Success - file selected and path stored in filePath
 * @retval 0 Cancelled - user cancelled the dialog
 * @retval -1 Error - dialog failed to open or invalid parameters
 *
 * @pre filePath buffer must be allocated with at least maxPathLength bytes
 * @pre maxPathLength must be greater than 0
 * @post If successful, filePath contains null-terminated path to selected file
 *
 * @note Uses Windows Common Dialog APIs (comdlg32.dll)
 * @warning filePath buffer must be large enough to hold the complete path
 * @warning Ensure maxPathLength does not exceed actual buffer size
 *
 * @see OpenWindowsFolderDialog() for folder selection
 */
int OpenWindowsFileDialog(char* filePath, int maxPathLength);

/**
 * @brief Opens Windows folder selection dialog
 *
 * Displays the standard Windows folder browse dialog allowing user to select
 * a directory. The selected folder path is returned in the provided buffer.
 *
 * @param[out] folderPath Buffer to store the selected folder path
 * @param[in] maxPathLength Maximum length of the folder path buffer
 *
 * @return int Status code
 * @retval 1 Success - folder selected and path stored in folderPath
 * @retval 0 Cancelled - user cancelled the dialog
 * @retval -1 Error - dialog failed to open or invalid parameters
 *
 * @pre folderPath buffer must be allocated with at least maxPathLength bytes
 * @pre maxPathLength must be greater than 0
 * @post If successful, folderPath contains null-terminated path to selected folder
 *
 * @note Uses Windows Shell APIs for folder browsing
 * @warning folderPath buffer must be large enough to hold the complete path
 * @warning Ensure maxPathLength does not exceed actual buffer size
 *
 * @see OpenWindowsFileDialog() for file selection
 * @see RemoveItemsRecurse() for folder deletion
 */
int OpenWindowsFolderDialog(char* folderPath, int maxPathLength);


/**
 * @brief Converts input path to full Windows absolute path
 *
 * Resolves relative paths, UNC paths, and path shortcuts to their full
 * absolute representation using Windows API. Dynamically allocates memory
 * to accommodate paths of any length up to system limits.
 *
 * @param[in] inputPath Input path to be converted (relative or absolute)
 *
 * @return char* Pointer to dynamically allocated full path string
 * @retval Non-NULL Success - returns pointer to full path string
 * @retval NULL Failure - invalid input path or memory allocation failed
 *
 * @pre inputPath must be a valid null-terminated string
 * @post If successful, returns dynamically allocated string with full path
 *
 * @note Uses GetFullPathNameA() Windows API for path resolution
 * @note Caller is responsible for freeing the returned memory using free()
 * @note Automatically handles buffer resizing for long paths
 *
 * @warning Returned pointer must be freed by caller to prevent memory leaks
 * @warning Returns NULL on failure - always check return value before use
 *
 * @internal This is a private helper function - not exposed in header file
 */
char* _getFullWindowsPath(const char* inputPath);

/**
 * @brief Validates Windows path accessibility and existence
 *
 * Performs comprehensive validation of a Windows file system path by checking
 * both existence and accessibility. Uses Windows API to verify the path exists
 * and can be accessed with read permissions.
 *
 * @param[in] path Windows file system path to validate
 *
 * @return int Validation result
 * @retval 1 Success - path exists and is accessible
 * @retval 0 Failure - path doesn't exist, invalid, or access denied
 *
 * @pre path parameter is not NULL
 * @post Path validation completed, no side effects on file system
 *
 * @note Uses GetFileAttributesA() to check existence
 * @note Uses CreateFileA() with GENERIC_READ to test accessibility
 * @note Handles both files and directories appropriately
 * @note Automatically closes file handles to prevent resource leaks
 *
 * @warning Returns 0 for NULL or empty path strings
 * @warning May fail for paths requiring elevated permissions
 *
 * @details Validation process:
 * 1. Check for NULL or empty path
 * 2. Verify path exists using GetFileAttributesA()
 * 3. Test read access using CreateFileA()
 * 4. Handle directories with FILE_FLAG_BACKUP_SEMANTICS
 * 5. Clean up resources properly
 *
 * @see _getFullWindowsPath() for path normalization
 * @see OpenWindowsFileDialog() for user path selection
 *
 * @since 1.0
 */
int ValidateWindowsPath(const char* path);

/**
 * @brief Retrieves Windows common folder paths
 *
 * Gets the full path to standard Windows user folders such as Desktop,
 * Documents, Downloads, etc. Uses the USERPROFILE environment variable
 * to construct appropriate paths for the current user.
 *
 * @param[in] folder Enumerated folder type to retrieve
 * @param[out] path Buffer to store the resulting folder path
 * @param[in] pathSize Maximum size of the path buffer in bytes
 *
 * @return int Operation result
 * @retval 1 Success - path retrieved and stored in buffer
 * @retval 0 Failure - invalid folder type or environment variable not found
 *
 * @pre path buffer must be allocated with at least pathSize bytes
 * @pre pathSize must be sufficient to hold the complete path
 * @post If successful, path contains null-terminated folder path string
 *
 * @note Uses USERPROFILE environment variable as base path
 * @note Logs generated path for debugging purposes
 * @note Path format uses Windows backslash separators
 *
 * @warning Ensure pathSize is large enough to prevent buffer overflow
 * @warning Returns 0 if USERPROFILE environment variable is not set
 *
 * @details Supported folder types:
 * - WIN_FOLDER_DESKTOP: User's Desktop folder
 * - WIN_FOLDER_DOCUMENTS: User's Documents folder
 * - WIN_FOLDER_DOWNLOADS: User's Downloads folder
 * - WIN_FOLDER_PICTURES: User's Pictures folder
 * - WIN_FOLDER_MUSIC: User's Music folder
 * - WIN_FOLDER_VIDEOS: User's Videos folder
 *
 * @example
 * @code
 * char desktopPath[MAX_PATH];
 * if (GetWindowsCommonPath(WIN_FOLDER_DESKTOP, desktopPath, MAX_PATH)) {
 *     printf("Desktop path: %s\n", desktopPath);
 * }
 * @endcode
 *
 * @see WindowsCommonFolder for available folder types
 * @see ValidateWindowsPath() for path validation
 *
 * @since 1.0
 */
int GetWindowsCommonPath(WindowsCommonFolder folder, char* path, int pathSize);

#endif