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
      bool rtn1 = fms->add("A.ulam","element A { typedef Int(4) Foo [8]; Foo foo(Bool b) { Foo m; if(b) m[0]=1; else m[0]=2; return m;} Int foo(Int i) { Foo e; e[3] = 3; e[4] = 4; e[0] = i; return i; } Int test() { Bool mybool; mybool= true;\nd = foo(mybool); d[0] = foo(6);\n return d[0]; /* match return type */}\nFoo d; }");  // note: overloaded foo has different return types, error!


      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3139_test_compiler_funcdef_overload_returntype)

} //end MFM
