#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3440_test_compiler_voidnonzerobitsize_error)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:12:1: ERROR: Invalid nonzero size (=3) for Void type.
      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\nconstant Void(3) c_noway = 0;\n  Void func(/*Void arg*/) {\n return;\n}\n Int test(){\n//Void me;\n /* func(me);\n*/ return 0;\n}\n}\n");

      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3440_test_compiler_voidnonzerobitsize_error)

} //end MFM
