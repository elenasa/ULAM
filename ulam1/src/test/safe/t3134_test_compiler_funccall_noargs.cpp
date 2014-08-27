#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3134_test_compiler_funccall_noargs)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int a(0);  Int test() {  ( )foo return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a; Int foo() { return 1; } Int test() { return foo(); } }");
            
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3134_test_compiler_funccall_noargs)
  
} //end MFM


