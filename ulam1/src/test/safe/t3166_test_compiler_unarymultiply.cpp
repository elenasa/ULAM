#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3166_test_compiler_unarymultiply)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Unary(3) a(2);  Unary(3) b(2);  Unary(3) c(3);  Int(32) test() {  a 3 cast = b 3 cast = c a b * = c cast return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //2 * 2 == 4 bit -> maxes out 3 bit unary == 3
      bool rtn1 = fms->add("A.ulam","element A { Unary(3) a, b, c; use test;  a = 3; b = 3; c = a * b; return c; } }");
      bool rtn2 = fms->add("test.ulam", "Int test() {");
      
      if(rtn1 & rtn2)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3166_test_compiler_unarymultiply)
  
} //end MFM


