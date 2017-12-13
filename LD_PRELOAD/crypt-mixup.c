#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>

static char *(*_crypt) (const char *key, const char *salt) = NULL;

static char *crypt_res = NULL;

char *crypt(const char *key, const char *salt) {

  if (_crypt == NULL) {
    _crypt = (char *(*)(const char *key, const char *salt)) dlsym(RTLD_NEXT, "crypt");
    crypt_res = NULL;
  }

  if (crypt_res == NULL) {
    crypt_res = _crypt(key, salt);
    return crypt_res;
  }

  _crypt = NULL;
  return crypt_res;
}
