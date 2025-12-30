<!-- ============================================
     AlpenliCloud - File Manager GUI
     Struktur Data & Algoritma - Tugas Akhir
     ============================================ -->

<!-- TEAM DAN TANGGUNG JAWAB -->
<!--
  Team 1 - Arif
    - Memperbaiki kesalahan dari tim 2 dan 3
    - Implementasi:
      void pasteFile(FileManager *fileManager);
      void copyFile(FileManager *fileManager);
      void cutFile(FileManager *fileManager);

  Team 2 - Farras
    - Implementasi:
      void createFile(FileManager *fileManager);
      void recoverFile(FileManager *fileManager);
      void deleteFile(FileManager *fileManager);

  Team 3 - Maul
    - Implementasi:
      void updateFile(FileManager *fileManager);
      void searchFile(FileManager *fileManager);
      void redo(FileManager *fileManager);
      void undo(FileManager *fileManager);
-->

<table border="0" cellspacing="0" cellpadding="0">
    <tr>
        <td style="vertical-align:bottom; width:60px; padding-right:10px; border:none;">
            <img src="assets/icon.png" style="height:50px; width:50px;">
        </td>
        <td style="vertical-align:middle; border:none;">
            <h1 style="margin:0;">AlpenliCloud</h1>
        </td>
    </tr>
</table>

![platform](https://img.shields.io/badge/platform-windows%20%7C%20macos%20%7C%20linux-2b6cb0)
![language](https://img.shields.io/badge/language-C-00599C)
![graphics](https://img.shields.io/badge/graphics-raylib%20%7C%20raygui-1f8a70)

Bahasa Inggris: lihat [README.md](README.md).

## Deskripsi Program

AlpenliCloud adalah aplikasi **File Manager** berbasis **Graphical User Interface (GUI)** yang dirancang menyerupai Windows Explorer. Aplikasi ini mengimplementasikan struktur data berbasis **Abstract Data Type (ADT)** seperti **non-binary tree**, **stack**, dan **queue** untuk merepresentasikan struktur dan perilaku sistem file secara digital, dengan antarmuka yang intuitif menggunakan raylib dan raygui.

Sekarang AlpenliCloud sudah **lintas OS**: Windows, macOS, dan Linux (dengan catatan dependensi raylib dan dialog file sesuai platform).

### Tujuan

Aplikasi ini dibuat untuk memenuhi Tugas Akhir mata kuliah **Struktur Data dan Algoritma**, dengan fokus pada penerapan konsep struktur data dalam konteks manajemen file. AlpenliCloud menyediakan fitur pengelolaan file dan folder seperti membuat, memperbarui, menghapus, memulihkan, menyalin, memindahkan, serta menampilkan file dengan antarmuka grafis yang user-friendly. Fitur **undo** dan **redo** juga didukung melalui struktur data **stack**.

### Fitur Utama

1. **Antarmuka GUI Modern**: Interface menyerupai Windows Explorer dengan title bar, toolbar, navbar, sidebar, dan body area
2. **File Manager Operations**:
    - Create file/folder dengan modal dialog
    - Rename file/folder (F2 shortcut)
    - Delete file/folder ke trash
    - Copy (Ctrl+C), Cut (Ctrl+X), Paste (Ctrl+V)
    - Buka file dengan aplikasi default
3. **Navigation System**:
    - Sidebar dengan tree structure yang dapat di-expand/collapse
    - Navbar dengan path navigation dan search functionality
    - Back navigation (Backspace/Alt+Left)
4. **Advanced Features**:
    - Progress bar untuk operasi file dalam jumlah banyak (>10 items)
    - Undo/Redo system dengan visual feedback
    - Multi-selection dengan Ctrl+Click dan Select All (Ctrl+A)
    - Keyboard shortcuts lengkap
5. **Trash Management**:
    - Soft delete ke trash directory
    - Recover files dari trash
    - Duplicate name handling
6. **Window Management**:
    - Resizable window dengan drag borders
    - Maximize/minimize buttons
    - Custom title bar

### Komponen GUI

-   **TitleBar**: Window controls (minimize, maximize, close) dan window dragging
-   **Navbar**: Path navigation, search box, dan navigation buttons (back, undo, redo)
-   **Toolbar**: File operations (New, Copy, Cut, Paste, Delete, Rename)
-   **Sidebar**: Tree view untuk navigasi folder hierarchy
-   **Body**: Main content area dengan file/folder listing dalam format tabel

## Struktur Data yang Digunakan

-   **Non-Binary Tree**: Untuk merepresentasikan struktur direktori dan file system hierarchy
-   **Stack**: Untuk mendukung fitur undo dan redo operations
-   **Queue**: Untuk mengelola copied/cut items dan paste operations
-   **LinkedList**: Untuk mengelola selected items dan trash items

## Teknologi yang Digunakan

-   **raylib**: Graphics library untuk rendering dan input handling
-   **raygui**: Immediate mode GUI library untuk UI components
-   **C Language**: Core programming language
-   **Platform APIs**: Untuk operasi file dan dialog sesuai sistem (Windows/macOS/Linux)

## Cara Menjalankan

1. Pastikan sistem mendukung kompilasi C dengan raylib
2. Clone repository ini:
    ```bash
    git clone https://github.com/riefproject/AlpenliCloud.git
    ```
3. Compile dan run:
    - Menggunakan build script:
        ```bash
        ./build.sh
        # Parameter tersedia: clean, build, rebuild
        # contoh: ./build.sh rebuild
        ```
    - Menggunakan debug script (dengan debug symbols):
        ```bash
        ./debug.sh
        ```
    - Menggunakan Makefile:
        ```bash
        make # optional parameter <clean | rebuild>
        ```

### Catatan Linux

- Build akan memakai `lib/raylib/lib/libraylib.so` jika tersedia.
- Jika tidak ada, build akan mencoba `pkg-config --cflags --libs raylib` dan memberi pesan untuk install raylib (`libraylib-dev`).
- Dialog file/folder memakai `zenity` atau `kdialog`. Jika belum ada, install salah satu.

## Cara Penggunaan

### Navigasi Dasar

-   **Double-click folder**: Masuk ke direktori
-   **Double-click file**: Buka dengan aplikasi default
-   **Sidebar navigation**: Klik folder di sidebar untuk navigasi cepat
-   **Path bar**: Ketik path manual untuk navigasi langsung

### File Operations

-   **New File/Folder**: Klik tombol "New" di toolbar atau Ctrl+N
-   **Copy**: Select file → Ctrl+C atau tombol Copy
-   **Cut**: Select file → Ctrl+X atau tombol Cut
-   **Paste**: Ctrl+V atau tombol Paste
-   **Delete**: Select file → Delete key atau tombol Delete
-   **Rename**: Select file → F2 atau tombol Rename

### Selection

-   **Single select**: Click pada file/folder
-   **Multi-select**: Ctrl+Click untuk toggle selection
-   **Select all**: Ctrl+A atau checkbox di header tabel
-   **Deselect all**: Click area kosong atau Ctrl+A lagi

### Keyboard Shortcuts

-   **Ctrl+C**: Copy selected items
-   **Ctrl+X**: Cut selected items
-   **Ctrl+V**: Paste items
-   **Ctrl+Z**: Undo last operation
-   **Ctrl+Y**: Redo operation
-   **F2**: Rename selected item
-   **F5/Ctrl+R**: Refresh view
-   **Delete**: Move to trash
-   **Backspace/Alt+Left**: Go back
-   **Ctrl+A**: Select/deselect all

## Struktur Project

```
AlpenliCloud/
├── src/
│   ├── main.c                 # Entry point aplikasi
│   ├── file_manager.c         # Core file management logic
│   ├── utils.c               # Utility functions dan shortcuts
│   ├── win_utils.c           # Windows-specific operations
│   ├── gui/
│   │   ├── ctx.c             # GUI context management
│   │   ├── titlebar.c        # Window title bar component
│   │   ├── navbar.c          # Navigation bar component
│   │   ├── toolbar.c         # Toolbar component
│   │   ├── sidebar.c         # Sidebar tree component
│   │   ├── body.c            # Main content area
│   │   └── component.c       # Reusable UI components
│   └── data_structure/       # ADT implementations
├── include/                  # Header files
├── lib/raylib/              # raylib library
├── assets/                  # Icons dan resources
├── build/                   # Build output
└── bin/                     # Executable output
```

## Kontribusi

Kontribusi sangat terbuka untuk pengembangan lebih lanjut. Silakan fork repository ini dan buat pull request untuk perbaikan atau penambahan fitur.

## Lisensi

Proyek ini menggunakan lisensi [MIT](LICENSE).

## Kontak

Untuk pertanyaan atau diskusi lebih lanjut, silakan hubungi melalui [GitHub Issues](https://github.com/riefproject/AlpenliCloud/issues).
