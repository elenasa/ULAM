#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3312_test_compiler_lvalintarray)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_A { Int(16) a[2](1,0);  Int(32) test() {  a 2 1 -b [] 1 cast = a 0 [] a 1 [] cast 0 +b cast = a a 0 [] [] a 0 [] cast 1 -b cast = a a 1 [] [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n Int(16) a[2];\n Int test() {\n a[2-1] = 1;\n a[0] = a[1] + 0;\n a[a[0]] = a[0] - 1;\n return a[a[1]];\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3312_test_compiler_lvalintarray)

} //end MFM
