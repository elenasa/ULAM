#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3141_test_compiler_castarg)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { typedef Int Foo[8];  Int d[8](1,0,0,0,0,0,0,0);  Int test() {  Int a;  a 1 = d ( a cast )foo = d 0 [] return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { typedef Int Foo [8]; Foo foo(Bool b) { Foo m; if(b) m[0]=1; else m[0]=2; return m;} Int test() { Int a; a = 1;\nd = foo((Bool) a);\n return d[0]; /* match return type */}\nFoo d; }");  // want d[0] == 1.

      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3141_test_compiler_castarg)
  
} //end MFM


