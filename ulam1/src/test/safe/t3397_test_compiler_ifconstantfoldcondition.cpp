#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3397_test_compiler_ifconstantfoldcondition)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_R { Int(32) test() {  true cond 3 return if 0 return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("R.ulam"," element R{\n Int test(){\nif(0 < 1)\n return 3;\n return 0;\n }\n}\n");

      if(rtn1 )
	return std::string("R.ulam");
      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3397_test_compiler_ifconstantfoldcondition)

} //end MFM
