#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3141_test_compiler_castarg)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_A { typedef Int(16) Foo[2];  Int(16) d[2](1,0);  Int(32) test() {  Int(32) a;  a 1 = d ( a 1 == )foo = d 0 [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n typedef Int(16) Foo [2];\n Foo foo(Bool b) {\n Foo m;\n if(b)\n m[0]=1;\n else\n m[0]=2;\n return m;\n}\n Int test() {\n Int a;\n a = 1;\nd = foo(a==1);\n return d[0];\n /* match return type */}\nFoo d;\n }\n");  // want d[0] == 1.


      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3141_test_compiler_castarg)

} //end MFM
