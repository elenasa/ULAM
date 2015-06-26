#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3221_test_compiler_bitwiseandequalwithconstant)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 2\nUe_A { typedef Bits(3) B;  Bits(3) b(2);  Int(32) test() {  b 2 cast = b 3 cast &= b cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // as Bits and a constant
      bool rtn1 = fms->add("A.ulam","ulam 1;\n element A {\ntypedef Bits(3) B;\n B b;\n Int test() {\nb = 2;\n b &= 3;\nreturn (Int) b;\n}\n}\n");

      // if constant doesn't fit..
      //./A.ulam:7:4: ERROR: Constant <8> is not representable as: Bits(3), for binary operator&= .
      //./A.ulam:7:4: ERROR: Bits is the supported type for bitwise operator&=.
      //bool rtn1 = fms->add("A.ulam","ulam 1;\n element A {\ntypedef Bits(3) B;\n B b;\n Int test() {\nb = 2;\n b &= 8;\nreturn (Int) b;\n}\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3221_test_compiler_bitwiseandequalwithconstant)

} //end MFM
