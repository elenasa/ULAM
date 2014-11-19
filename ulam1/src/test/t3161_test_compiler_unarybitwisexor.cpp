#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3161_test_compiler_unarybitwisexor)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Unary(3) a(2);  Unary(3) b(1);  Unary(3) c(1);  Int(3) e(2);  Int(3) f(1);  Int(3) g(3);  Int(32) test() {  a e 2 cast = cast = b f 1 cast = cast = c a b ^ = g e f ^ = c cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // a is 2 bits; b is 1 bit; a | b = 2 bit; notice as Int.3 (g), 1 | 2 = 3
      bool rtn1 = fms->add("A.ulam","element A { Unary(3) a, b, c; Int(3) e, f, g; use test;  a = e = 2; b = f = 1; c = a ^ b; g = e ^ f; return c; } }");
      bool rtn2 = fms->add("test.ulam", "Int test() {");
      
      if(rtn1 & rtn2)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3161_test_compiler_unarybitwisexor)
  
} //end MFM


