#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3467_test_compiler_bitsshift)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Bits(3) u(6);  Bits(3) v(3);  Int(32) test() {  v 7u cast = u v 1 cast << = v v 1 cast >> = 0 return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\nBits(3) u, v;\n Int test() {\n v = 7u;\n u = v << 1;\n v = v >> (2 - 1);\n return 0;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3467_test_compiler_bitsshift)

} //end MFM
