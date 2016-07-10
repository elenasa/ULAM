#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3494_test_compiler_divideandmodmixedtypes)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_A { Unary(4) e(3);  Unary(4) c(0);  Int(4) k(2);  Int(4) j(1);  Int(32) test() {  Int(32) g;  g 11 cast = Int(32) h;  h 2 cast = e 3u cast = k g e cast % cast = j e cast h / cast = j cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //i picked lhs of equal so no casting would be asked for!
      bool rtn1 = fms->add("A.ulam","element A {Unary(4) e,c;\nInt(4) k;\n Int(4) j;\nuse test;\n Int g = 11;\n Int h = 2;\n e = 3u;\n k = (g % e);\n j = e / h;\n return j;\n } }");

      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      if(rtn1 && rtn2)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3494_test_compiler_divideandmodmixedtypes)

} //end MFM
