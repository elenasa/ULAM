#include "TestCase.h"

namespace MFM {

  BEGINTESTCASE(t10_testfoo)
  {
    bool RunTests(File * errorOutput)
    {
      // Do some stuff right here to run the tests
      // return true if all okay.
      // output messaes to errorOutput and return false
      // if anything goes wrong
      errorOutput->write("something bad happened.");
      return false;
    }
  }
  
  ENDTESTCASE(t10_testfoo)
  
  //#undef FOO
} //end MFM


