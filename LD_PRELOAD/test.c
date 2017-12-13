#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  char passwd[] = "foobar";

  if (argc < 2) {
    printf("usage: %s <given-password>\n", argv[0]);
    return 0;
  }

  if (!strcmp(passwd, argv[1])) {
    printf("Green light!\n");
    return 1;
  }

  printf("Red light!\n");
  return 0;
}
