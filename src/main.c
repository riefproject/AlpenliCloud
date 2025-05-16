#include <stdio.h>
#include <dirent.h>

#include <sys/stat.h>

int main ()
{
  DIR *dp;

  struct dirent *ep;     
  dp = opendir("./");

  if (dp != NULL) {
    ep = readdir(dp);

    while (ep != NULL) {
      printf("%s\n", ep->d_name);
      ep = readdir(dp);
    }

    closedir(dp);
    return 0;
  } else {
    perror ("Couldn't open the directory");
    return -1;
  }
}

// Team 1 arif
// Memperbaiki kesalahan tim 2 dan 3
// void pasteFile(FileManager *fileManager);
// void copyFile(FileManager *fileManager);
// void cutFile(FileManager *fileManager);

// Team 2 farras
// void createFile(FileManager *fileManager);
// void recoverFile(FileManager *fileManager);
// void deleteFile(FileManager *fileManager);

// Team 2 maul
// void updateFile(FileManager *fileManager);
// void searchFile(FileManager *fileManager);
// void redo(FileManager *fileManager);
// void undo(FileManager *fileManager);