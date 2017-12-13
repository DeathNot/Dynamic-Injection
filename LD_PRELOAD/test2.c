#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char **argv) {
  char  buf[256], alpha[34], beta[34];
  int   j, plen, fd;

  if (argc < 2) {
    printf("usage: %s <keyfile>\n", argv[0]);
    return 1;
  }

  if (strlen(argv[1]) > 256) {
    fprintf(stderr, "keyfile length is > 256, go fish!\n");
    return 0;
  }

  fd = open(argv[1], O_RDONLY);

  if (fd < 0) {
    perror(argv[1]);
    return 0;
  }

  memset(buf, 0x0, sizeof(buf));

  plen = read(fd, buf, strlen(argv[1]));

  if (plen != strlen(argv[1])) {

    if (plen < 0) {
      perror(argv[1]);
    }

    printf("Sorry!\n");
    return 0;
  }

  strncpy(alpha, (char *)crypt(argv[1], "$1$"), sizeof(alpha));
  strncpy(beta, (char *)crypt(buf, "$1$"), sizeof(beta));

  for(j = 0; j < strlen(alpha); j++) {

    if (alpha[j] != beta[j]) {

      printf("Sorry!\n");
      return 0;
    }
  }

  printf("All your base are belong to us!\n");
  return 1;
}
