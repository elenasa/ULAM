#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3284_test_compiler_varcalledself_error)
  {
    std::string GetAnswerKey()
    {
      //./Su.ulam:5:6: ERROR: The keyword 'self' may not be used as a variable name.
      return std::string("Ue_Su { 0 cast return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //should return a parse error.
      bool rtn1 = fms->add("Su.ulam", "ulam 1;\nelement Su {\nInt test(){\n{\n Int self;\n}\nreturn 0;\n}\n}\n");

      if(rtn1)
	return std::string("Su.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3284_test_compiler_varcalledself_error)

} //end MFM
