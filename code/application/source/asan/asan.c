#include <stdio.h>
#include <stdlib.h>
int main() {
  char *x = (char*)malloc(10 * sizeof(char*));
  free(x);
  printf("x[5]=0x%x\n", x[5]);

  return 0;
}