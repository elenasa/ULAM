#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3431_test_compiler_missingmember_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:5:15: ERROR: Undetermined type for missing member 'long' Proxy.
      return std::string("Exit status: 64\nUe_A { Int(32) test() {  typedef Int(64) Long;  Int(64) k;  Int(64) m;  m k 9223372036854775807 +b = 64u cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //infomed by t3428
      // typo long.maxof..
      bool rtn1 = fms->add("A.ulam"," element A {\n Int test(){\ntypedef Int(64) Long;\n Long k,m;\n m = k + long.maxof;\n return m.sizeof;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3431_test_compiler_missingmember_error)

} //end MFM
