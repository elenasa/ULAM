#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3441_test_compiler_constantdefmissingequals_error)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:3:25: ERROR: Missing '=' after named constant definition.
      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\nconstant Void(3) c_noway;\n  Void func(/*Void arg*/) {\n return;\n}\n Int test(){\n//Void me;\n /* func(me);\n*/ return 0;\n}\n}\n");

      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3441_test_compiler_constantdefmissingequals_error)

} //end MFM
