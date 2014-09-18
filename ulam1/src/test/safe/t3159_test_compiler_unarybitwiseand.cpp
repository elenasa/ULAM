#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3159_test_compiler_unarybitwiseand)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Unary(3) a(2);  Unary(3) b(1);  Unary(3) c(1);  Int(3) e(3);  Int(3) f(4);  Int(3) g(0);  Int(32) test() {  a e 3 cast = cast = b f 4 cast = cast = c a b & = g e f & = c cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // a is 2 bits; b is 1 bit; a & b = 1 bit; notice as Int.3 (g), 4 & 3 = 0
      bool rtn1 = fms->add("A.ulam","element A { Unary(3) a, b, c; Int(3) e, f, g; use test;  a = e = 3; b = f = 4; c = a & b; g = e & f; return c; } }");
      bool rtn2 = fms->add("test.ulam", "Int test() {");
      
      if(rtn1 & rtn2)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3159_test_compiler_unarybitwiseand)
  
} //end MFM


