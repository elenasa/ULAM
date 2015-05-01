#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3326_test_compiler_elementwithclassparameters)
  {
    std::string GetAnswerKey()
    {
      //Constants have explicit types
      return std::string("Exit status: 3\nUe_R { Int(32) test() {  Foo(3) e;  Foo(4) f;  f a . 3 cast = f a . cast return } }\nUe_Foo { constant Int(32) n = NONREADYCONST;  Unsigned(UNKNOWN) a(0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo(Int n){\nUnsigned(n) a;\n}");

      bool rtn2 = fms->add("R.ulam","ulam 1;\nuse Foo;\n element R{\n Int test() {\nFoo(3) e;\n Foo(4) f;\n f.a = 3;\n return f.a;\n }\n }\n");
      //simplified to debugging, only one instance, no cast for assignment;
      //bool rtn2 = fms->add("R.ulam","ulam 1;\nuse Foo;\n element R{\nInt test() {\nFoo(4) f;\nf.a = 3u;\n return 0;\n }\n }\n");

      if(rtn1 && rtn2)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3326_test_compiler_elementwithclassparameters)

} //end MFM
