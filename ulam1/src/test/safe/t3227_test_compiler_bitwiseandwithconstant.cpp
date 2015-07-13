#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3227_test_compiler_bitwiseandwithconstant)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 2\nUe_A { typedef Int(3) I;  typedef Bits(3) B;  Int(3) a(3);  Int(3) b(2);  Int(32) test() {  a 3 = b a cast 2 cast & cast = b cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // as Bits and a constant
      //bool rtn1 = fms->add("A.ulam","ulam 1;\n element A {\ntypedef Int(3) I;\n typedef Bits(3) B;\n B a, b;\n Int test() {\na = 3;\nb = a & 2;\nreturn (Int) b;\n}\n}\n");

      // as a Int cast to Bits and a constant
      bool rtn1 = fms->add("A.ulam","ulam 1;\n element A {\ntypedef Int(3) I;\n typedef Bits(3) B;\n I a, b;\n Int test() {\na = 3;\nb = (I) (a & 2);\nreturn b;\n}\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3227_test_compiler_bitwiseandwithconstant)

} //end MFM
