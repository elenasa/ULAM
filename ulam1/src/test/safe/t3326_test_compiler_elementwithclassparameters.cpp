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
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo(Int n){\nUnsigned(n) a;\nInt test() {\nFoo(3) e;\n Foo(4) f;\nf.a = 3;\n return a;\n }\n }\n"); //error, can't return template data member
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo(Int n){\nUnsigned(n) a;\nInt test() {\nFoo(3) e;\n Foo(4) f;\nf.a = 3;\n return 0;\n }\n }\n");

      //simplified to debugging, only one instance, no cast for assignment;
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo(Int n){\nUnsigned(n) a;\nInt test() {\nFoo(4) f;\nf.a = 3u;\n return 0;\n }\n }\n");

      if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3326_test_compiler_elementwithclassparameters)

} //end MFM


