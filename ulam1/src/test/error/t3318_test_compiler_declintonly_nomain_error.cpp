#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3318_test_compiler_declintonly_nomain_error)
  {
    std::string GetAnswerKey()
    {
      //return std::string(" a x +b PI *\n2\n");
      return std::string("Ue_A { Int(32) a(0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A{\nInt a;\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3318_test_compiler_declintonly_nomain_error)

} //end MFM


