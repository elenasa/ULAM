#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3156_test_compiler_unaryadd)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Unary(3) a(2);  Unary(3) b(1);  Unary(3) c(3);  Int(32) test() {  a 2 cast = b 1 cast = c a cast b cast +b cast = c cast return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Unary(3) a, b, c; use test;  a = 2; b = 1; c = a + b; return c; } }");
      bool rtn2 = fms->add("test.ulam", "Int test() {");
      
      if(rtn1 & rtn2)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3156_test_compiler_unaryadd)
  
} //end MFM


