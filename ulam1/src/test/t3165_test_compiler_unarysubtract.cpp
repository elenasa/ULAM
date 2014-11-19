#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3165_test_compiler_unarysubtract)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Unary(3) a(2);  Unary(3) b(1);  Unary(3) c(1);  Unary(3) d(0);  Int(32) test() {  a 2 cast = b 1 cast = c a cast b cast -b cast = d b cast a cast -b cast = c cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //2 - 1 == 1 bit; for negative unary, d is 0
      bool rtn1 = fms->add("A.ulam","element A { Unary(3) a, b, c, d; use test;  a = 2; b = 1; c = a - b; d = b - a; return c; } }");
      bool rtn2 = fms->add("test.ulam", "Int test() {");
      
      if(rtn1 & rtn2)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3165_test_compiler_unarysubtract)
  
} //end MFM


