<!-- SEMENTARA AJA INI -->
<!-- KALO ADA YANG GA SESUAI EDIT AJA -->

<table border="0" cellspacing="0" cellpadding="0">
    <tr>
        <td style="vertical-align:bottom; width:60px; padding-right:10px; border:none;">
            <img src="assets/logo.ico" style="height:50px; width:50px;">
        </td>
        <td style="vertical-align:middle; border:none;">
            <h1 style="margin:0;">AlpenliCloud</h1>
        </td>
    </tr>
</table>

## Deskripsi Program

AlpenliCloud adalah aplikasi **File Manager** berbasis **Command Line Interface (CLI)** yang dirancang sebagai simulasi sistem manajemen berkas sederhana. Aplikasi ini mengimplementasikan struktur data berbasis **Abstract Data Type (ADT)** seperti **non-binary tree**, **stack**, dan **queue** untuk merepresentasikan struktur dan perilaku sistem file secara digital.

### Tujuan

Aplikasi ini dibuat untuk memenuhi Tugas Akhir mata kuliah **Struktur Data dan Algoritma**, dengan fokus pada penerapan konsep struktur data dalam konteks manajemen file. AlpenliCloud menyediakan fitur pengelolaan file dan folder seperti membuat, memperbarui, menghapus, memulihkan, menyalin, memindahkan, serta menampilkan file. Fitur **undo** dan **redo** juga didukung melalui struktur data **stack**, memberikan pengalaman interaktif tanpa antarmuka grafis.

### Fitur Utama

1. **Inisialisasi File Manager**: Menginisialisasi struktur data (Tree, Stack, Queue) untuk pengelolaan file.
2. **Create File**: Membuat file atau folder baru sebagai child dari node parent.
3. **Update File**: Mengubah nama file atau folder.
4. **Delete File**: Memindahkan file/folder ke trash.
5. **Recover File**: Memulihkan file/folder dari trash ke direktori utama.
6. **Search File**: Mencari file berdasarkan nama dengan traversal Tree.
7. **Undo Aksi**: Membatalkan aksi terakhir menggunakan stack.
8. **Redo Aksi**: Mengembalikan aksi yang telah di-undo.
9. **Copy File**: Menyalin file ke buffer sementara.
10. **Paste File**: Menambahkan salinan file dari buffer ke direktori tujuan.
11. **Cut File**: Memindahkan file/folder ke lokasi baru.
12. **Show File**: Menampilkan struktur direktori atau trash secara hierarkis.

## Struktur Data yang Digunakan

-   **Non-Binary Tree**: Untuk merepresentasikan struktur direktori dan trash.
-   **Stack**: Untuk mendukung fitur undo dan redo.
-   **Queue**: Untuk mengelola item yang sedang dipilih.

## Cara Menjalankan

1.  Pastikan lingkungan pengembangan mendukung bahasa pemrograman yang digunakan (misalnya, C, C++, atau lainnya).
2.  Clone repository ini:
    ```bash
    git clone https://github.com/riefproject/AlpenliCloud.git
    ```
3.  Compile and run: - Menggunakan shell script:
    ````bash
    . build.sh
        # Anda dapat menambahkan parameter clean atau rebuild jika dibutuhkan
        # contoh: . build.sh rebuild
        # atau tanpa parameter sekalipun
        ```
        - Menggunakan Makefile
        ```bash
        make
        ```
    ````

## Contoh Penggunaan

Berikut adalah contoh beberapa perintah yang dapat dijalankan di AlpenliCloud CLI:

```bash
# Membuat folder baru
create_folder Documents

# Membuat file baru di dalam folder
create_file Documents tugas.txt

# Mengubah nama file
update_file Documents/tugas.txt tugas_akhir.txt

# Menghapus file (memindahkan ke trash)
delete_file Documents/tugas_akhir.txt

# Memulihkan file dari trash
recover_file tugas_akhir.txt

# Menyalin file
copy_file Documents/tugas_akhir.txt

# Menempelkan file ke folder lain
paste_file Backup

# Memindahkan file ke folder lain
cut_file Documents/tugas_akhir.txt
paste_file Backup

# Menampilkan struktur direktori
show_tree

# Menampilkan isi trash
show_trash

# Undo aksi terakhir
undo

# Redo aksi yang telah di-undo
redo
```

## Kontribusi

Kontribusi sangat terbuka untuk pengembangan lebih lanjut. Silakan fork repository ini dan buat pull request untuk perbaikan atau penambahan fitur.

## Lisensi

Proyek ini menggunakan lisensi [MIT](LICENSE).

## Kontak

Untuk pertanyaan atau diskusi lebih lanjut, silakan hubungi melalui [GitHub Issues](https://github.com/riefproject/AlpenliCloud/issues).
