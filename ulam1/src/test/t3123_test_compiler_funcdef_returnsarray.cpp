#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3123_test_compiler_funcdef_returnsarray)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { typedef Int Foo[8];  Int d[8](1,0,0,0,0,0,0,0);  Int main() {  Bool mybool;  mybool true = d ( mybool )foo = d 0 [] } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { typedef Int Foo [8]; Foo foo(Bool b) { Foo m; if(b) m[0]=1; else m[0]=2; m;} Int main() { Bool mybool; mybool= true;\nd = foo(mybool);\nd[0]; /* match return type */}\nFoo d; }");  // want d[0] == 1.
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3123_test_compiler_funcdef_returnsarray)
  
} //end MFM


