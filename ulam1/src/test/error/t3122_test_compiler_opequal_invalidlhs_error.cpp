#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3122_test_compiler_opequal_invalidlhs_error)
  {
    std::string GetAnswerKey()
    {
      //./D.ulam:1:36: ERROR: Invalid Statement (possible missing semicolon).
      return std::string("Ue_D { Int a[2](1,0);  Int test() {  a 0 [] 1 = } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","element D { Int a[2]; Int test() { 3 = a[1]; a[0] = 1; return a[1]; } }");

      if(rtn1)
	return std::string("D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3122_test_compiler_opequal_invalidlhs_error)

} //end MFM
