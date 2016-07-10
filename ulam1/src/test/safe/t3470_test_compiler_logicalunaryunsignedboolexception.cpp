#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3470_test_compiler_logicalunaryunsignedboolexception)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_A { Unary(1) a(1);  Unsigned(1) b(1);  Int(32) test() {  a b 1 cast = cast = a cast b cast && cond a cast return if b cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // no casts required for unary(1) and unsigned(1): return smaller
      bool rtn1 = fms->add("A.ulam","element A {\nUnary(1) a;\n Unsigned(1) b;\n Int test() {\n a = b = 1; if(a && b) return a;\n return b;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3470_test_compiler_logicalunaryunsignedboolexception)

} //end MFM
