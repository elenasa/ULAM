#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3439_test_compiler_constantdefvoid_error)
  {
    std::string GetAnswerKey()
    {
      //./Tu.ulam:3:10: ERROR: Invalid use of type: Void(0) used with named constant symbol name 'c_noway'.
      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\nconstant Void c_noway = 0;\n  Void func(/*Void arg*/) {\n return;\n}\n Int test(){\n//Void me;\n /* func(me);\n*/ return 0;\n}\n}\n");

      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3439_test_compiler_constantdefvoid_error)

} //end MFM
