#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3495_test_compiler_oneplusoneequalstwo)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 2\nUe_A { Int(4) j(2);  Int(4) k(0);  Int(4) m(1);  Int(4) n(1);  Int(32) test() {  j 2 cast = k 0 cast = m 1 = n 1 cast = j cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //i picked lhs of equal so no casting would be asked for!
      bool rtn1 = fms->add("A.ulam","element A {Int(4) j,k,m,n;\nuse test;\n j = 1 + 1;\n k = 1 - 1;\n m = 1 * 1;\n n = 1 / 1;\nreturn j;\n } }");

      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      if(rtn1 && rtn2)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3495_test_compiler_oneplusoneequalstwo)

} //end MFM
