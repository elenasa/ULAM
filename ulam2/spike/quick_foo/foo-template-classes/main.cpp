#include "Foo.h"
#include "Bar.h"

int main() {
  Foo<3> f;
  f.foomem = 320;
  Bar<3> b;
  b.member = 9;
  b.bfunc(f);
  f.func(b);
  return 0;
}
