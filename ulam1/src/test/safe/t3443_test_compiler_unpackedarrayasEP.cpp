#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3443_test_compiler_unpackedarrayasEP)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 512\nUe_A { typedef Int(32) BigSite[10];  Int(32) test() {  sep 2 [] 512 = sep 2 [] return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\ntypedef Int BigSite[10];\n element BigSite sep;\n Int test(){\n sep[2] = 512;\n return sep[2];\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3443_test_compiler_unpackedarrayasEP)

} //end MFM
