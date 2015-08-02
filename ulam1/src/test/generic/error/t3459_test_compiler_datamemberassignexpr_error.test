#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3459_test_compiler_datamemberassignexpr_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:2:26: ERROR: Cannot assign to data member <cprint_name> at the time of its declaration.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3457, except data member
      bool rtn1 = fms->add("A.ulam","element A{\n Unsigned(8) cprint_name = ;\n Int test () {\n  return cPRINT_NAME;\n}\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3459_test_compiler_datamemberassignexpr_error)

} //end MFM
