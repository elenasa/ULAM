#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3139_test_compiler_funcdef_overload_returntype)
  {
    std::string GetAnswerKey()
    {
      //no longer an error
      return std::string("Exit status: 6\nUe_A { typedef Int(4) Foo[8];  Int(4) d[8](6,0,0,0,0,0,0,0);  Int(32) test() {  Bool(1) mybool;  mybool true = d ( mybool )foo = d 0 [] ( 6 )foo cast = d 0 [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n typedef Int(4) Foo [8];\n Foo foo(Bool b) {\n Foo m;\n if(b) m[0]=1;\n else m[0]=2;\n return m;\n} Int foo(Int i) {\n Foo e;\n e[3] = 3;\n e[4] = 4;\n e[0] = (Int(4)) i;\n return i;\n } Int test() {\n Bool mybool;\n mybool= true;\nd = foo(mybool);\n d[0] = (Int(4)) foo(6);\n return d[0]; /* match return type */\n}\nFoo d;\n }\n");  // note: overloaded foo has different return types, error!


      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3139_test_compiler_funcdef_overload_returntype)

} //end MFM
