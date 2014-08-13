#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3107_test_compiler_minusequal)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(-1);  Int b(2);  Int main() {  a 1 = b 2 = a b -= } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a, b; Int main() {a = 1; b = 2; a-=b;} }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3107_test_compiler_minusequal)
  
} //end MFM


