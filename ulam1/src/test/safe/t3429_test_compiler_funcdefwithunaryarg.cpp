#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3429_test_compiler_funcdefwithunaryarg)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_Foo { Bool(1) b(true);  Unary(7) s(3);  Int(32) test() {  s 3u cast = b self ( s )check . = s cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3405
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\n element Foo {\nBar(3) bar;\n Int test() {\nBool bs = bar.check();\nreturn bs;\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam","ulam 1;\n quark Bar(Unary(7) s) {\nBool b;\n Bool check(){\n b = (s == 7);\n return b;\n}\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3429_test_compiler_funcdefwithunaryarg)

} //end MFM
