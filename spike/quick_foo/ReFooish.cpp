// g++ -O99 -Wall -pedantic -ansi ReFooish.cpp -o ReFooish;./ReFooish
// g++ -g2 -Wall -pedantic -ansi ReFooish.cpp -o ReFooish;./ReFooish
#include <stdlib.h>  /* for abort() */
/* 
   Determine if a given address lies in the local variable space of
   the current function's stack frame.

   Note this method may return true for some addresses that don't
   actually contain local variables, due to stack alignment padding or
   who-knows-what.  But we are only concerned with addresses that _do_
   land in the storage space of local variables, and we don't mind some
   false positives nearby.  

   (At least, that's what we're hoping for.)

 */
inline bool _IsLocal(void * address) __attribute__ ((always_inline));

bool _IsLocal(void * address) 
{
  // Here we assume the stack grows down.  Can arrange to establish
  // the stack direction during runtime startup if that's uncertain.
  void * highest = __builtin_frame_address(0);

  if (highest == 0)
  {
    // 0 apparently amounts to some kind of error condition from
    // __builtin_frame_address
    abort();
  }

  // Ugh, this is machine-specific.  We are accessing the stack
  // pointer (in processor register 'esp', in this case) to find the
  // lower bound on the current stack frame.  Now, conceivably, culam
  // might be able to know this directly itself, because any and all
  // variables in the local stack frame were put there by culam.  But
  // that's's at least non-obvious, depending on how gcc lays out
  // scoped local variables and so forth.  But if that was possible we
  // could maybe avoid the asm here.

  register void * lowest asm("esp"); 

  // OK, finally the actual test.  If the given address is between
  // lowest and highest, we say it _IsLocal in the current stack frame.
  return address >= lowest && address <= highest;
}


// A grody hacro for tests..
#include <stdio.h>
#define TESTLOC(varname,answer)               \
do {                                          \
  bool _a_ = _IsLocal((void*) &(varname));    \
  printf("%s is%s local", #varname, _a_?"":" NOT"); \
  if (_a_ == (answer)) printf(" (correct)\n"); \
  else printf(" WRONG!\n%s:%d: Previous _IsLocal failed\n", \
              __FILE__, __LINE__);                          \
 } while (0)

int globe;
int zot;

void func1() {
  TESTLOC(globe,false);
  bool glebe;
  TESTLOC(glebe,true);
  int globe;
  TESTLOC(globe,true);

  int & ll = zot;     // So ll is not local, because zot is global here?
  TESTLOC(ll, false); // Yes!  Even though 'int& ll' itself _is_ a
                      // local variable!  (Ow my head!)

  TESTLOC(zot,true);    // TEST: should fail at runtime (zot isn't local here)
}

int main() {
  int zot;
  TESTLOC(zot,true);
  func1();
  {
    TESTLOC(globe,false);
    bool glebe;
    TESTLOC(glebe,true);
    TESTLOC(zot,true);
    int globe;
    TESTLOC(globe,true);
  }
  TESTLOC(globe,false);
  return 0;
}

