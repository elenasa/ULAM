#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3418_test_compiler_namedconstant_unsignedchar)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 26\nUe_A { constant Unsigned(8) cPRINT_NAME = 26;  Int(32) test() {  26u cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3414, except constant type is unsigned char
      bool rtn1 = fms->add("A.ulam","element A{\nconstant Unsigned(8) cPRINT_NAME = 'z' - 'a' + 1;\n Int test () {\n return cPRINT_NAME;\n}\n}\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3418_test_compiler_namedconstant_unsignedchar)

} //end MFM
