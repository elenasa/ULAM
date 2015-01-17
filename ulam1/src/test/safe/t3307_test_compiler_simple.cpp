#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3307_test_compiler_simple)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 8\nUe_A { Int(16) a[2](4,8);  Int(32) test() {  a 0 [] 1 3 +b cast = a 1 [] 2 4 * cast = a 1 [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n Int(16) a[2];\n Int test() {\n a[0] = 1 + 3;\n a[1] = 2 * 4;\n return a[1];\n }\n }\n"); // simple case

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3307_test_compiler_simple)

} //end MFM


