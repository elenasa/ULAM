#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3301_test_compiler_controlwhile)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 8\nUe_A { Int(32) a(0);  Int(32) b(8);  Int(32) test() {  a 5 = b 0 = a 1 -= 0 != cond b b 2 +b = _1: while b return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
            bool rtn1 = fms->add("A.ulam","element A {\n Int a, b;\n Int test() {\n a = 5;\n b = 0;\n while( --a != 0 )\n b = b + 2;\n return b;\n }\n }\n");
	    //bool rtn1 = fms->add("A.ulam","element A {\n Int a, b;\n Int test() {\n a = 5;\n b = 0;\n while( --a )\n b += 2;\n return b;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3301_test_compiler_controlwhile)

} //end MFM
