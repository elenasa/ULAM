#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3310_test_compiler_lval)
  {
    std::string GetAnswerKey()
    {
      //constant fold: a 2 1 -b
      return std::string("Exit status: 3\nUe_A { Int(16) a[2](3,7);  Int(32) test() {  Int(32) j;  a 1 [] 7 cast = j 10 cast a 1 [] -b cast = a 0 [] j cast = j return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n Int(16) a[2];\n Int test() {\n Int j;\n a[2-1] = 7;\n j = 10 - a[1];\n a[0] = j;\n return j;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3310_test_compiler_lval)

} //end MFM
