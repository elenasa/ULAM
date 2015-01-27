#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3326_test_compiler_elementwithclassparameters)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_Foo { Bool(7) sp(false);  Int(32) a(0);  Int(32) test() {  Foo f;  1 cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo(Int n){\nUnsigned(n) a;\nInt test() {\n Foo(4) f;\n  return 1;\n }\n }\n");

      if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3326_test_compiler_elementwithclassparameters)

} //end MFM


