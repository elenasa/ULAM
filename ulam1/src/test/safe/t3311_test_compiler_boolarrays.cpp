#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3311_test_compiler_boolarrays)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Bool(1) a[5](false,false,false,false,true);  Int(32) test() {  a 1 3 +b [] true = 0 return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n Bool a[5];\n Int test() {\n a[1+3] = true;\n return 0;\n}\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3311_test_compiler_boolarrays)

} //end MFM
