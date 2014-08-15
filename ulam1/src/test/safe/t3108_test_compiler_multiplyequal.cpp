#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3108_test_compiler_multiplyequal)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(6);  Int b(2);  Int test() {  a 3 = b 2 = a b *= a return } }\nExit status: 6");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a, b; Int test() {a = 3; b = 2; a*=b; return a; } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3108_test_compiler_multiplyequal)
  
} //end MFM


