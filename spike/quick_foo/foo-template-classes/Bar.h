#ifndef BAR_H
#define BAR_H

template<int POS>
class Foo; // forward

template<int POS>
struct Bar {
  int member;
  void bfunc(Foo<POS> f) ;
};

template<int POS>
void Bar<POS>::bfunc(Foo<POS> f) {
  printf("bf %d %d\n", POS, f.foomem);
}

#endif /* BAR_H */
