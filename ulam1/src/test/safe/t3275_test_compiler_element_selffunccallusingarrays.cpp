#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3275_test_compiler_element_selffunccallusingarrays)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Foo { Bool(1) b[3](true,true,false);  Int(32) test() {  b 0 [] true = b 1 [] self ( 0 cast )check . = 0 cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\nBool b[3];\nBool check(Int idx){\n return b[idx];\n}\n Int test() {\nb[0] = true;\nb[1] = self.check(0);\nreturn 0;\n }\n }\n");

      if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3275_test_compiler_element_selffunccallusingarrays)

} //end MFM
