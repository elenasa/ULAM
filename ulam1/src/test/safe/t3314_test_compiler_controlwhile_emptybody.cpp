#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3314_test_compiler_controlwhile_emptybody)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { Bool(7) b(false);  Int(32) a(0);  Int(32) test() {  a 5 = a a 1 -b = cast cond {} _1: while a return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\nBool(7) b;\n Int a;\n Int test() {\na = 5;\n while( a = a - 1 ) { }\n return a;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3314_test_compiler_controlwhile_emptybody)

} //end MFM
