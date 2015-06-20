#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3464_test_compiler_quarkshift_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:6:7: ERROR: (2) <toInt> is not a defined function, and cannot be called; compiling class: A.
      //./A.ulam:7:8: ERROR: (2) <toInt> is not a defined function, and cannot be called; compiling class: A.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use Bar;\n element A {\n Int b, c;\n Int test() {\n Bar a;\nb = 1 << a;\n c = a << 2;\n return 0;\n }\n }\n");

      //quark has no toInt method:
      bool rtn2 = fms->add("Bar.ulam","quark Bar {\n  }\n");

      if(rtn1 && rtn2)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3464_test_compiler_quarkshift_error)

} //end MFM
