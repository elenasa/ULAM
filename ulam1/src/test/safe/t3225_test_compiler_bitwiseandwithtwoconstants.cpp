#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3225_test_compiler_bitwiseandwithtwoconstants)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 2\nUe_A { typedef Int(3) I;  Int(3) b(2);  Int(32) test() {  b 2u cast = b cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // as two constants (see t3227 for one constant)
      bool rtn1 = fms->add("A.ulam","ulam 1;\n element A {\ntypedef Int(3) I;\n I b;\n Int test() {\nb = (I) (3 & 2);\nreturn b;\n}\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3225_test_compiler_bitwiseandwithtwoconstants)

} //end MFM
