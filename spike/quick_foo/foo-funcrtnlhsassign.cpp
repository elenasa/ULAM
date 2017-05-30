#include <stdio.h>
#include <stdlib.h>

class foo {

public:
  foo() : m_dm(1) {}
  ~foo() {}

  foo func() {
    return *this;
  }

  foo funcloc() {
    foo f;
    return f;
  }

private:
  int m_dm;
};


int main()
{
  foo f, f1;
  f.func() = f1;
  f.funcloc() = f1;
  return 0;
}
