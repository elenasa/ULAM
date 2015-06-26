#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3456_test_compiler_atommodelparameter_error)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:4:17: ERROR: Missing '=' after model parameter definition.
      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // cannot write into an MP in ulam (only MFM simulator!)
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\ntypedef Atom A;\n parameter A aep;\n  Int test(){\n aep = self;\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3456_test_compiler_atommodelparameter_error)

} //end MFM
