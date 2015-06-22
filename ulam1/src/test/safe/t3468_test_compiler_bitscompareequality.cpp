#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3468_test_compiler_bitscompareequality)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Bool(1) a(false);  Bool(1) b(true);  Bits(3) u(1);  Bits(3) v(7);  Int(32) test() {  v 7u cast = u 9u cast = a u v == = b u v != = u 7u cast == cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //Bits do not saturate!!
      bool rtn1 = fms->add("A.ulam","element A {\nBool a,b;\n Bits(3) u, v;\n Int test() {\n v = 7u;\n u = (Bits(3)) 9u;\na = (u == v);\n b = (u != v);\n return u == 7u;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3468_test_compiler_bitscompareequality)

} //end MFM
