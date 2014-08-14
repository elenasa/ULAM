#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3137_test_compiler_funcdef_returnstatements_error)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { typedef Int Foo[8];  Int d[8](1,0,0,0,0,0,0,0);  Int test() {  Bool mybool;  mybool true = d ( mybool )foo = d 0 [] } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { typedef Int Foo [8]; Foo foo() { Foo m; m[0]=1; return m[0];} Int test() {d = foo();\nreturn d[0]; /* match return type */}\nFoo d; }");  // want d[0] == 1. simplified from original; needs 1 return, 1 differ in arraysize

      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3137_test_compiler_funcdef_returnstatements_error)
  
} //end MFM


