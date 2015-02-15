#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3326_test_compiler_elementwithclassparameters)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 3\nExit status: 3\nUe_Foo { constant Int(CONSTANT) n = NONREADYCONST;  Unsigned(UNKNOWN) a(0);  Int(32) test() {  Foo e;  Foo f;  f a . 3 = f a . return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo(Int n){\nUnsigned(n) a;\nInt test() {\nFoo(3) e;\n Foo(4) f;\nf.a = 3;\n return f.a;\n }\n }\n"); //error, can't return template data member <a>

      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo(Int n){\nUnsigned(n) a;\nInt test() {\nFoo(3) e;\n Foo(4) f;\nf.a = 3u;\n return 0;\n }\n }\n");

      //simplified to debugging, only one instance, cast for assignment;
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo(Int n){\nUnsigned(n) a;\nInt test() {\n Foo(4) f;\nf.a = 3;\n return 0;\n }\n }\n");

      //simplified to debugging, only one instance, no cast for assignment;
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo(Int n){\nUnsigned(n) a;\nInt test() {\nFoo(4) f;\nf.a = 3u;\n return 0;\n }\n }\n");

      if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3326_test_compiler_elementwithclassparameters)

} //end MFM


