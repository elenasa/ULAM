#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3463_test_compiler_quarkshift)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Int(32) b(2);  Int(32) c(4);  Int(32) test() {  Bar a;  b 1 a ( )toInt . cast cast << = c a ( )toInt . cast 2 cast << = 0 return } }\nUq_Bar { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use Bar;\n element A {\n Int b, c;\n Int test() {\n Bar a;\nb = 1 << a;\n c = a << 2;\n return 0;\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam","quark Bar {\n Int toInt() { return 1;\n}\n }\n");

      if(rtn1 && rtn2)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3463_test_compiler_quarkshift)

} //end MFM
