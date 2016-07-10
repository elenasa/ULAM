#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3126_test_compiler_typedef_scope)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 4\nUe_A { Int(32) x(4);  Int(32) test() {  typedef Int(32) Bar[2];  Int(32) e[2];  { e 0 [] 4 cast = } x e 0 [] = x return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // needs newlines
      bool rtn1 = fms->add("A.ulam","element A {\n Int x;\n Int test() {\n typedef Int Bar[2];\n Bar e;\n {\n e[0] = 4;\n }\n /* match int return type */\n x= e[0];\n return x;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3126_test_compiler_typedef_scope)

} //end MFM
