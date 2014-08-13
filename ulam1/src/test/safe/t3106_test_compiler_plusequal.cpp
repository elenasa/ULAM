#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3106_test_compiler_plusequal)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(3);  Int b(2);  Int main() {  a 1 = b 2 = a b += } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a, b; use main;  a = 1; b = 2; a+=b; } }");
      bool rtn2 = fms->add("main.ulam", "Int main() {");
      
      if(rtn1 & rtn2)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3106_test_compiler_plusequal)
  
} //end MFM


