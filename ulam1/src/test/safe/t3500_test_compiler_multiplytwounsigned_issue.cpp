#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3500_test_compiler_multiplytwounsigned_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_A { typedef Unsigned(4) Timer;  Int(32) test() {  Unsigned(4) t;  Unsigned(8) res;  res t cast t cast * = ; res cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //i picked lhs of equal so no casting would be asked for!
      // twice around the c&l loop added r+ l for multiply.
      bool rtn1 = fms->add("A.ulam","element A {typedef Unsigned(4) Timer;\nuse test;\n Timer t;\n Unsigned(8) res = t * t;\n;\nreturn res;\n } }");

      bool rtn2 = fms->add("test.ulam", "Int test() {\n");

      if(rtn1 && rtn2)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3500_test_compiler_multiplytwounsigned_issue)

} //end MFM
