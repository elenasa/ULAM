#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3414_test_compiler_namedconstantbits)
  {
    std::string GetAnswerKey()
    {
      //Bits exception for fits test (eventhough doesn't allowminmax).
      return std::string("Exit status: 0\nUe_A { constant Bits(3) cPRINT_NAME = 2;  constant Bits(3) cPRINT_VALUE = 1;  constant Bits(3) cPRINT_ALL = 3;  Int(32) test() {  0 return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3324, except constant type is 3-bits only
      bool rtn1 = fms->add("A.ulam","element A{\nconstant Bits(3) cPRINT_NAME = 2;\n constant Bits(3) cPRINT_VALUE = 1;\n constant Bits(3) cPRINT_ALL = cPRINT_NAME | cPRINT_VALUE;\nInt test () {\n return 0;\n}\n}\n");
      //bool rtn1 = fms->add("A.ulam","element A{\nconstant Unsigned(3) cPRINT_NAME = 2;\n constant Unsigned(3) cPRINT_VALUE = 1;\n constant Unsigned(3) cPRINT_ALL = cPRINT_NAME | cPRINT_VALUE;\nInt test () {\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3414_test_compiler_namedconstantbits)

} //end MFM
