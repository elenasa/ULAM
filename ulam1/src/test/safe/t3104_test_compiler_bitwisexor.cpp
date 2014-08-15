#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3104_test_compiler_bitwisexor)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(3);  Int b(2);  Int c(1);  Bool d(true);  Int test() {  a 3 = b 2 = d c a b ^ = cast = c return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a, b, c; Bool d; use test;  a = 3; b = 2; d = c = a ^ b; return c; } }");
      bool rtn2 = fms->add("test.ulam", "Int test() {");

      if(rtn1 & rtn2)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3104_test_compiler_bitwisexor)
  
} //end MFM


