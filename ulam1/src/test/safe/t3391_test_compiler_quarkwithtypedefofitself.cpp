#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3391_test_compiler_quarkwithtypedefofitself)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Typo { typedef Typo Self;  Int(32) test() {  0 return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Typo.ulam", "ulam 1;\nelement Typo {\ntypedef Typo Self;\n Int test() {\n return 0;\n}\n }\n");

      if(rtn1)
	return std::string("Typo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3391_test_compiler_quarkwithtypedefofitself)

} //end MFM
