#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3254_test_compiler_ifblockscope_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 10\nUe_E { Int(32) test() {  E e;  Int(32) a;  a 10 cast = e ( a )func . true cond { Int(28) a;  a 6 cast = } if e ( a )func . a return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("E.ulam"," ulam 1;\n element E{\n Void func(Int d) { }\n Int test() {\n E e;\n Int a = 10;\n e.func(a);  // works\n if (true) {\n Int(28) a = 6;\n }\n e.func(a);  // fails\n return a;\n }\n }\n");

      if(rtn1)
	return std::string("E.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3254_test_compiler_ifblockscope_issue)

} //end MFM


