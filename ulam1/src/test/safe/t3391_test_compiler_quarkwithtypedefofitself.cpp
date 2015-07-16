#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3391_test_compiler_quarkwithtypedefofitself)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_ElTypo { Typo t( typedef Typo Self; );  Int(32) test() {  0 cast return } }\nUq_Typo { typedef Typo Self;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //      bool rtn1 = fms->add("Typo.ulam", "ulam 1;\nelement Typo {\ntypedef Typo Self;\n Int test() {\n return 0;\n}\n }\n");
      bool rtn2 = fms->add("ElTypo.ulam", "ulam 1;\nuse Typo;\n element ElTypo {\nTypo t;\n Int test() {\n return 0;\n}\n }\n");

      bool rtn1 = fms->add("Typo.ulam", "ulam 1;\nquark Typo {\ntypedef Typo Self;\n }\n");

      if(rtn1 && rtn2)
	return std::string("ElTypo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3391_test_compiler_quarkwithtypedefofitself)

} //end MFM
