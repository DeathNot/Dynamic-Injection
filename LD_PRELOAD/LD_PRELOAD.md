# 利用LD_PRELOAD机制进行动态注入
LD_PRELOAD是linux下的一个环境变量，动态链接器在载入一个程序所需的所有动态库之前，首先会载入LD_PRELOAD环境变量所指定的动态库。运用这个机制，可以修改已有动态库中的方法，加入我们需要的逻辑，改编程序的执行行为。此方法只对动态链接的程序有效，静态链接程序无效。
测试程序如下
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
编译方式如下：
```
gcc -fPIC -c strcmp-hijack.c -o strcmp-hijack.o
gcc -shared -o strcmp-hijack.so strcmp-hijack.o
```
现在我们如果简单执行./test，结果如下：
```
./test redbull
Red light
```
若果在之前我们自己实现的动态链接库，结果如下：
```
LD_PRELOAD="./strcmp-hijack.so" ./test redbull
s1 eq foobar
s2 eq redbull
Green light!
```
可以看到，现在程序跳到了passwd正确的逻辑。










参考文献：https://www.exploit-db.com/papers/13233/
