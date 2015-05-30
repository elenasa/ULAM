#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3421_test_compiler_namedconstant_unsignedchar_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:2:37: ERROR: Unexpected input!! Token: <ContinueError>.
      return std::string("Exit status: 26\nUe_A { constant Unsigned(8) cPRINT_NAME = 26;  Int(32) test() {  26u cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3418, except constant type is tictic
      bool rtn1 = fms->add("A.ulam","element A{\nconstant Unsigned(8) cPRINT_NAME = '';\n Int test () {\n return cPRINT_NAME;\n}\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3421_test_compiler_namedconstant_unsignedchar_error)

} //end MFM
