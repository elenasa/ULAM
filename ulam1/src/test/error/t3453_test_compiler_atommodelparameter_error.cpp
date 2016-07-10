#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3453_test_compiler_atommodelparameter_error)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:6:12: ERROR: Constant value expression for: aep, is not a constant, type: <Empty(UNKNOWN)<14>>
      //./Tu.ulam:6:12: ERROR: Constant value expression for: aep, is not a constant, type: <Empty(0)<14>>.
      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\n use Empty;\n element Tu {\ntypedef Atom A;\n Empty e;\n parameter A aep = e;\n  Int test(){\n return 0;\n}\n}\n");

      bool rtn2 = fms->add("Empty.ulam", "ulam 1;\n element Empty {\n }\n");

      if(rtn1 && rtn2)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3453_test_compiler_atommodelparameter_error)

} //end MFM
