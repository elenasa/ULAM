#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3138_test_compiler_funcdef_overload)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { typedef Int Foo[2];  Int d[2](6,3);  Int test() {  Bool mybool;  mybool true = d ( mybool )foo = d ( 6 )foo = d 0 [] return } }\nExit status: 6");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { typedef Int Foo [2]; Foo foo(Bool b) { Foo m; if(b) m[0]=1; else m[0]=2; return m;} Foo foo(Int i) { Foo e; e[1] = 3; e[0] = i; return e; } Int test() { Bool mybool; mybool= true;\nd = foo(mybool); d = foo(6);\n return d[0]; /* match return type */}\nFoo d; }");  // want d[0] == 6. 

      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3138_test_compiler_funcdef_overload)
  
} //end MFM


