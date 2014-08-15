#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3134_test_compiler_funccall_noargs)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(0);  Int test() {  ( )foo return } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a; Int foo() { return 1; } Int test() { return foo(); } }");
            
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3134_test_compiler_funccall_noargs)
  
} //end MFM


