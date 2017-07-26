#include <stdio.h>
 
int main(int argc, char **argv) {
  if (argc!=2) {
    printf("Usage : strlen <string>\n");
  } else {
    printf("Size: %d    \"%s\"\n\n", strlen(argv[1]), argv[1]);
  }

  return 0 ;
}
