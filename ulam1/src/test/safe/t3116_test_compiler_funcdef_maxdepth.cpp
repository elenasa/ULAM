#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3116_test_compiler_funcdef_maxdepth)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Bool c(true);  Int main() {  c ( 1 true )foo = 0 } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Bool foo(Int m, Bool b) { Int d; { Int e[8]; b;} Bool c; c = d = m; c; } Int main() { c = foo(1, true); 0; } Bool c; }");  // max depth is 9;
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3116_test_compiler_funcdef_maxdepth)
  
} //end MFM


