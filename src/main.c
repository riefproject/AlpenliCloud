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