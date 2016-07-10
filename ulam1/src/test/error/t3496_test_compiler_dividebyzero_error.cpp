#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3496_test_compiler_dividebyzero_error)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 2\nUe_A { Int(4) j(2);  Int(4) k(0);  Int(4) m(1);  Int(4) n(1);  Int(32) test() {  j 2 cast = k 0 cast = m 1 = n 1 cast = j cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //i picked lhs of equal so no casting would be asked for!
      bool rtn1 = fms->add("A.ulam","element A {Int(4) n;\nuse test;\n n = 1 / 0;\nreturn -1;\n } }");

      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      if(rtn1 && rtn2)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3496_test_compiler_dividebyzero_error)

} //end MFM
