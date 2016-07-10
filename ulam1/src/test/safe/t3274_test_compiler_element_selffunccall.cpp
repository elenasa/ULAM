#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3274_test_compiler_element_selffunccall)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Foo { Bool(1) b(true);  Int(32) test() {  b true = Bool(1) c;  c self ( 0 cast )check . = 0 cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\nBool b;\nBool check(Int idx){\n return b;\n}\n Int test() {\nb = true;\nBool c = self.check(0);\nreturn 0;\n }\n }\n");

      if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3274_test_compiler_element_selffunccall)

} //end MFM
