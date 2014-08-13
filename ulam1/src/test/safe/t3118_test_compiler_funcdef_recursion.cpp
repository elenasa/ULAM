#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3118_test_compiler_funcdef_recursion)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(24);  Int main() {  a ( 4 )fact = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int main() { a = fact(4); } Int a; Int fact(Int n) { Int a; if(n) a = n * fact(n-1); else a = 1; a; }  }");  // 6
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3118_test_compiler_funcdef_recursion)
  
} //end MFM


