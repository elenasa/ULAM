#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3440_test_compiler_voidelementparameter_error)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:4:10: ERROR: Invalid use of type: Void(0) used with variable symbol name 'vep'.
      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\ntypedef Void V0;\n element V0 vep;\n  Int test(){\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3440_test_compiler_voidelementparameter_error)

} //end MFM
