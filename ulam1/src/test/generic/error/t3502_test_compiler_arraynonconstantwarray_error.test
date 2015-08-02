#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3502_test_compiler_arraynonconstantwarray_error)
  {
    std::string GetAnswerKey()
    {
      /*
	A.ulam:5:7: ERROR: Array size specifier in [] is not a constant number.
      */

      return std::string("Ue_A { Int(32) a(7);  Int(32) test() {  a 7 = } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3320
      bool rtn1 = fms->add("A.ulam","element A { constant Int a = 7;\n Int test() {\n Int c[1];\n c[0] = 5;\n Int b[a+c[0]];\n b[4] = 2; return b[4]; } }");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3502_test_compiler_arraynonconstantwarray_error)

} //end MFM
