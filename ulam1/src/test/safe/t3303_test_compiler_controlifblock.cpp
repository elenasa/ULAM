#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3303_test_compiler_controlifblock)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Int(32) a(6);  Int(32) b(0);  Int(32) test() {  a 7 = b 0 = a 0 != cond { Int(32) b;  b 1 b +b = a a 1 -b = } if b a = else b return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n Int a, b;\n Int test() {\na = 7;\n b = 0;\n if( a != 0 ){\n Int b; b = 1 + b;\n a = a - 1;\n }\n else\n b = a;\n return b;\n}\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3303_test_compiler_controlifblock)

} //end MFM
