#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3313_test_compiler_vardecls)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_A { Int(32) c(1);  Int(32) test() {  Int(32) a[2];  Int(32) b;  b 1 cast = a 0 cast b +b [] b = c a 1 [] = a 1 [] return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n Int c;\n Int test() {\n Int a[2], b;\n b = 1;\n a[0+b] = b;\n c = a[1];\n return a[1];\n }\n }\n"); //not error?

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3313_test_compiler_vardecls)

} //end MFM
