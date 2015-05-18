#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3119_test_compiler_funccall_undefined_error)
  {
    std::string GetAnswerKey()
    {
      //./D.ulam:1:30: ERROR: (2) <fact> is not a defined function, and cannot be called; compiling class: D.
      return std::string(" D.ulam:1:25: ERROR: (2) <fact> is not a defined function, and cannot be called.\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","element D { Int test() { a = fact(3); return a; } Int a;  }");  // 6

      if(rtn1)
	return std::string("D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3119_test_compiler_funccall_undefined_error)

} //end MFM
