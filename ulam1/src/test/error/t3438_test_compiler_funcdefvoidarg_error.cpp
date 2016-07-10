#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3438_test_compiler_funcdefvoidarg_error)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:3:12: ERROR: Invalid use of type: Void(0) used with variable symbol name 'arg'.
      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\n Void func(Void arg) {\n return;\n}\n Int test(){\n//Void me;\n /* func(me);\n*/ return 0;\n}\n}\n");

      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3438_test_compiler_funcdefvoidarg_error)

} //end MFM
