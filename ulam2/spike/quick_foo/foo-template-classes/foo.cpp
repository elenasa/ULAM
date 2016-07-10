#include <stdio.h>

class Bar; // forward
struct Foo {
  void func(Bar b) ;
  int foomem;
};

struct Bar {
  int member;
  void bfunc(Foo f) ;
};

void Foo::func(Bar b) {
  printf("%d\n", b.member);
}

void Bar::bfunc(Foo f) {
  printf("bf %d\n",f.foomem);
}

int main() {
  Foo f;
  f.foomem = 320;
  Bar b;
  b.member = 9;
  b.bfunc(f);
  f.func(b);
  return 0;
}
