#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3107_test_compiler_minusequal)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) a(-1);  Int(32) b(2);  Int(32) test() {  a 1 = b 2 = a b -= a return } }\nExit status: -1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a, b; Int test() {a = 1; b = 2; a-=b; return a; } }");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3107_test_compiler_minusequal)
  
} //end MFM


