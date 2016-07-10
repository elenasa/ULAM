#include <stdio.h>

template<int POS>
class Bar; // forward

template <int POS>
struct Foo {
  Bar<POS> bar1;
  Bar<POS+3> bar2;
  void func(Bar<POS> b) ;
  int foomem;
};

template<int POS>
struct Bar {
  int member;
  void bfunc(Foo<POS> f) ;
};

template<int POS>
void Foo<POS>::func(Bar<POS> b) {
  printf("%d %d\n", POS, b.member);
}

template<int POS>
void Bar<POS>::bfunc(Foo<POS> f) {
  printf("bf %d %d\n", POS, f.foomem);
}

int main() {
  Foo<3> f;
  f.foomem = 320;
  Bar<3> b;
  b.member = 9;
  b.bfunc(f);
  f.func(b);
  return 0;
}
