/* --IOmatch-file-pattern : a failed open is ignored, and the close of the file
   can trigger an action which stops the instrumentation */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main (int argc, char** argv) {
  int failed=open ("iomatch-missing-directory/file.res", O_WRONLY);
  int fd=open ("iomatch-file.res", O_WRONLY|O_CREAT|O_TRUNC, 0644);
  if (write (fd, "line\n", 5)!=5) {
    return 1;
  }
  close (fd);
  printf ("failed=%d\n", failed);
  return 0;
}
