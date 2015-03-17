#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3128_test_compiler_decllist_witharray)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_A { Int(32) a(3);  Int(32) test() {  Int(32) b;  Int(32) c[2];  b 1 = b 2 += c 1 [] b = a c 1 [] = b return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //bool rtn1 = fms->add("A.ulam","element A { Int a; use test;  b = 1; b+=2; c[1] = b; a = c[1]; return b;} }");
      //needs newlines
      bool rtn1 = fms->add("A.ulam","element A {\n Int a;\n use test;\n  b = 1;\n b+=2;\n c[1] = b;\n a = c[1];\n return b;\n}\n }\n");
      bool rtn2 = fms->add("test.ulam", "Int test() {\n Int b, c[2];\n ");

      if(rtn1 & rtn2)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3128_test_compiler_decllist_witharray)

} //end MFM
