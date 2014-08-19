#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3140_test_compiler_funcdef_overload_plusequalarray_returntype)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { typedef Int Foo[8];  Int d[8](7,0,0,3,4,0,0,0);  Int test() {  Bool mybool;  mybool true = d ( mybool )foo = d ( 6 )foo += d 0 [] return } }\nExit status: 7");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { typedef Int Foo [8]; Foo foo(Bool b) { Foo m; if(b) m[0]=1; else m[0]=2; return m;} Foo foo(Int i) { Foo e; e[3] = 3; e[4] = 4; e[0] = i; return e; } Int test() { Bool mybool; mybool= true;\nd = foo(mybool); d += foo(6);\n return d[0]; /* match return type */}\nFoo d; }");  // want d[0] == 7. NOTE: requires 'operator+=' in c-code to add arrays

      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3140_test_compiler_funcdef_overload_plusequalarray_returntype)
  
} //end MFM


