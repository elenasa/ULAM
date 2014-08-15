#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3118_test_compiler_funcdef_recursion)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(24);  Int test() {  a ( 4 )fact = return } }\nExit status: 24");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int test() { return a = fact(4); } Int a; Int fact(Int n) { if(n) return (n * fact(n-1)); else return 1; }  }");  
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3118_test_compiler_funcdef_recursion)
  
} //end MFM


