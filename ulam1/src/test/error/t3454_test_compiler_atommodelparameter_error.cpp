#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3454_test_compiler_atommodelparameter_error)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:5:18: ERROR: Missing model parameter definition after '=' for 'aep'.
      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\n use Empty;\n element Tu {\ntypedef Atom A;\n parameter A aep = Empty;\n  Int test(){\n return 0;\n}\n}\n");

      bool rtn2 = fms->add("Empty.ulam", "ulam 1;\n element Empty {\n }\n");

      if(rtn1 && rtn2)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3454_test_compiler_atommodelparameter_error)

} //end MFM
