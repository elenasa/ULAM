#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3109_test_compiler_bitwiseandequal)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int a(2);  Int b(2);  Int test() {  a 3 = b 2 = a b &= a return } }\nExit status: 2");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a, b; Int test() {a = 3; b = 2; a&=b; return a; } }");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3109_test_compiler_bitwiseandequal)
  
} //end MFM


