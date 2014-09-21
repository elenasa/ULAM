#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3103_test_compiler_bitwiseor)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) a(3);  Int(32) b(2);  Bool(1) d(true);  Int(32) test() {  a 1 cast = b 2 cast = d a a b | = cast = a return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a, b; Bool d; use test;  a = 1; b = 2; d = a = a | b; return a; } }");
      bool rtn2 = fms->add("test.ulam", "Int test() {");
      
      if(rtn1 & rtn2)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3103_test_compiler_bitwiseor)
  
} //end MFM


