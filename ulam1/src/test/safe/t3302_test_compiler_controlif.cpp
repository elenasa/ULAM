#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3302_test_compiler_controlif)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Int(32) a(0);  Int(32) b(2);  Int(32) test() {  a 1 = b 0 = a a 1 -b = cast cond b 1 = if b 2 = else a return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n Int a, b;\n Int test() {\n a = 1;\n b = 0;\n if( a = a - 1 )\n b = 1;\n else\n b = 2;\n return a;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3302_test_compiler_controlif)

} //end MFM
