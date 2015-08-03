#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3321_test_compiler_arrayoutofbounds_error)
  {
    std::string GetAnswerKey()
    {
      //A.ulam:2:75: ERROR: Array subscript [2] exceeds the size (2) of array 'a'.


      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","ulam 1;\n element A{ Int(8) a[2]; Int test() { a[0] = 1 + 3; a[1] = 2 * 4; return a[2]; } }");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3321_test_compiler_arrayoutofbounds_error)

} //end MFM


