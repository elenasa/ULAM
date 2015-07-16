#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3138_test_compiler_funcdef_overload)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 6\nUe_A { typedef Int(16) Foo[2];  Int(16) d[2](6,3);  Int(32) test() {  Bool(1) mybool;  mybool true = d ( mybool )foo = d ( 6 cast )foo = d 0 [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n typedef Int(16) Foo [2];\n Foo foo(Bool b) {\n Foo m;\n if(b) m[0]=1;\n else m[0]=2;\n return m;\n} Foo foo(Int i) {\n Foo e;\n e[1] = 3;\n e[0] = (Int(16)) i;\n return e;\n } Int test() {\n Bool mybool;\n mybool= true;\nd = foo(mybool);\n d = foo(6);\n return d[0]; /* match return type */\n}\nFoo d;\n }\n");  // want d[0] == 6.


      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3138_test_compiler_funcdef_overload)

} //end MFM
