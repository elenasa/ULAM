#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3322_test_compiler_compound)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_A { Bar bar( Bool(1) b(true); );  Int(32) test() {  bar b . true = bar b . cast return } }\nUq_Bar { Bool(1) b(true);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","quark Bar{Bool b;\n }\n element A {\nBar bar;\n Int test() {\nbar.b = true;\nreturn bar.b;\n }\n }\n"); // compound case

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3322_test_compiler_compound)

} //end MFM
