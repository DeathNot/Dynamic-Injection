# 利用LD_PRELOAD机制进行动态注入
LD_PRELOAD是linux下的一个环境变量，动态链接器在载入一个程序所需的所有动态库之前，首先会载入LD_PRELOAD环境变量所指定的动态库。运用这个机制，可以修改已有动态库中的方法，加入我们需要的逻辑，改变程序的执行行为。此方法只对动态链接的程序有效，静态链接程序无效。

测试程序如下:
```
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

  printf("Red light\n");
  return 0;
}
```
编译
```
gcc test.c -o test
```
该程序是对输入的字符串与源程序中的passwd通过strcmp函数进行对比，现在我们自己实现一个strcmp函数，如下：
```
#include <stdio.h>
#include <string.h>

int strcmp(const char *s1, const char *s2) {

  printf("s1 eq %s\n", s1);
  printf("s2 eq %s\n", s2);
  return 0;
}
```
编译：
```
gcc -fPIC -c strcmp-hijack.c -o strcmp-hijack.o
gcc -shared -o strcmp-hijack.so strcmp-hijack.o
```
现在我们如果简单执行./test，结果如下：
```
./test redbull
Red light!
```
如果在之前加入我们自己实现的动态链接库，结果如下：
```
LD_PRELOAD="./strcmp-hijack.so" ./test redbull
s1 eq foobar
s2 eq redbull
Green light!
```
可以看到，现在程序跳到了passwd正确的逻辑。

接下来让我们试着注入一个稍微复杂点的程序，测试程序如下：
```
#include <stdio.h>
#include <unistd.h>
#include <sys/type.h>
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

```
编译时，必须加入-lcrypt选项，不然编译器会报错
```
gcc -o test2 test2.c -lcrypt
```
我们自己创建一个mykey，并在里面输入"Hello world!"
```
echo "Hello world!" > mykey
```
运行程序，结果如下：
```
./test2 mykey
Sorry!
```
现在我们思考一下如何注入，该程序首先是对filename进行一次hash，然后从filename文件中取出相应字节数进行一次hash，并最终对两次结果进行比较，由于程序中没有使用strcmp之类的函数，所以我们需要自己找程序的薄弱点，这个就在于crypt()函数，我们可以计算第一次的hash值，然后篡改第二次调用时得到的值，使之相等，就可以简单的通过比较测试。

下面是我们的代码实现
```
#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>

static char *(*_crypt) (const char *key, const char *salt) = NULL;

static char *crypt_res = NULL;

char *crypt(const char　*key, const char *salt) {

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
```
编译
```
gcc -fPIC -c -o crypt-mixup.o crypt-mixup.c
gcc -shared -o crypt-mixup.so crypt-mixup.o -ldl
```
执行结果如下：
```
LD_PRELOAD="./crypt-mixup.so" ./test2 mykey
All your base are belong to us!
```
现在程序已经进入比较成功的流程中了。

参考文献：https://www.exploit-db.com/papers/13233/
