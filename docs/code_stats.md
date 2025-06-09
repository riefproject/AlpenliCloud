# Code Statistics - AlpenliCloud

> Code complexity analysis generated using [Lizard](https://github.com/terryyin/lizard)

## File Statistics

### `src/file_manager.c` - Core File Manager

-   **2,026 NLOC** | 84 functions | Avg CCN: 5.3
-   The heart of AlpenliCloud - handles file operations, navigation, clipboard, and undo/redo functionality

### `src/win_utils.c` - Windows Integration

-   **231 NLOC** | 10 functions | Avg CCN: 5.3
-   Windows-specific file operations and dialogs

### `src/utils.c` - Utilities

-   **211 NLOC** | 4 functions | Avg CCN: 21.8
-   Input handling and keyboard shortcuts

### `src/item.c` - Item Management

-   **51 NLOC** | 3 functions | Avg CCN: 2.0
-   File/folder item creation and search

### `src/operation.c` - Operations

-   **11 NLOC** | 1 function | Avg CCN: 1.0
-   Operation tracking for undo/redo

## Warning Functions (High Complexity)

| Function            | File           | CCN    | NLOC | Issue                     |
| ------------------- | -------------- | ------ | ---- | ------------------------- |
| `ShortcutKeys`      | utils.c        | **79** | 172  | Extremely high complexity |
| `loadTrashFromFile` | file_manager.c | **24** | 104  | Complex file parsing      |
| `pasteFile`         | file_manager.c | **24** | 122  | Complex paste operations  |
| `importFile`        | file_manager.c | **18** | 83   | Complex import logic      |
| `_redoPaste`        | file_manager.c | **17** | 107  | Complex redo operations   |

## Summary

-   **Total NLOC**: 2,530 lines
-   **Total Functions**: 102
-   **Average CCN**: 5.8 (Good)
-   **Average Function Size**: 24.3 NLOC
-   **Warning Functions**: 5 (need refactoring)

The codebase is generally well-structured with moderate complexity. The `ShortcutKeys` function requires significant refactoring due to its extremely high cyclomatic complexity.

---

_Analysis generated with Lizard on June 9, 2025_
