#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3116_test_compiler_funcdef_maxdepth)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Bool(1) c(true);  Int(32) test() {  c ( 1 true )foo = c cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Bool foo(Int m, Bool b) { Int d; { Int e[8]; b;} Bool c; c = d = m; return c; } Int test() { c = foo(1, true); return c; } Bool c; }");  // max depth is 9; should cast return to Int.
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3116_test_compiler_funcdef_maxdepth)
  
} //end MFM


