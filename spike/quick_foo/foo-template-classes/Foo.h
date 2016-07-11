#ifndef FOO_H
#define FOO_H

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
void Foo<POS>::func(Bar<POS> b) {
  printf("%d %d\n", POS, b.member);
}


#endif /* FOO_H */
