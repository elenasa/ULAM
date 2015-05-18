#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3405_test_compiler_funcdefwithbitsarg_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nUe_Foo { Bool(1) b(true);  Int(32) test() {  Bits(32) s;  s 3u cast = b self ( s )check . = s cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\nBool b;\nBool check(Bits idx){\n return true;\n}\n Int test() {\nBits s = 3u;\nb = self.check(s);\nreturn s;\n }\n }\n");

      if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3405_test_compiler_funcdefwithbitsarg_issue)

} //end MFM
