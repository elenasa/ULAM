#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3111_test_compiler_bitwisexorequal)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(1);  Int b(2);  Int test() {  a 3 = b 2 = a b ^= a return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a, b; Int test() {a = 3; b = 2; a^=b; return a; } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3111_test_compiler_bitwisexorequal)
  
} //end MFM


