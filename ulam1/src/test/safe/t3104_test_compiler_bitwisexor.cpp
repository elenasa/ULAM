#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3104_test_compiler_bitwisexor)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int a(1);  Int b(2);  Bool d(true);  Int test() {  a 3 = b 2 = d a a b ^ = cast = a return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a, b; Bool d; use test;  a = 3; b = 2; d = a = a ^ b; return a; } }");
      bool rtn2 = fms->add("test.ulam", "Int test() {");

      if(rtn1 & rtn2)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3104_test_compiler_bitwisexor)
  
} //end MFM


