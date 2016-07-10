#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3437_test_compiler_funccallvoidarg_error)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:4:1: ERROR: Invalid use of type: Void(0) used with variable symbol name 'me'.
      //./Tu.ulam:5:3: ERROR: (2) <func> is not a defined function, and cannot be called; compiling class: Tu.
      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\n Int test(){\nVoid me;\n  func(me);\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3437_test_compiler_funccallvoidarg_error)

} //end MFM
