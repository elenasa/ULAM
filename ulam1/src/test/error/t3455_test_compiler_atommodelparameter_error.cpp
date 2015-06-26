#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3455_test_compiler_atommodelparameter_error)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:4:20: ERROR: (2) <self> is not defined, and cannot be used with class: Tu.
      // ./Tu.ulam:4:12: ERROR: Constant value expression for: aep, is invalid.

      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\ntypedef Atom A;\n parameter A aep = self;\n  Int test(){\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3455_test_compiler_atommodelparameter_error)

} //end MFM
