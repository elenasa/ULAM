#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3458_test_compiler_missingassignexpr_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:3:26: ERROR: RHS of binary operator= is missing, Assignment deleted.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3457, except local variable type
      bool rtn1 = fms->add("A.ulam","element A{\n Int test () {\n Unsigned(8) cprint_name = ;\n return cPRINT_NAME;\n}\n}\n");


      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3458_test_compiler_missingassignexpr_error)

} //end MFM
