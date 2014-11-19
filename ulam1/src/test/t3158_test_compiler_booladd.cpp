#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3158_test_compiler_booladd)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Bool(3) a(true);  Bool(3) b(false);  Bool(3) c(true);  Unary(3) d(1);  Int(32) test() {  a true cast = b false cast = c a cast b cast +b cast = d c cast = c cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //here's what happens when we try to add bools and save in a bool; 
      // note1: cast as a unary the sum is 3 bits (Bool.3 true)
      // note2: cast as an Int(32), the exit status is 1
      bool rtn1 = fms->add("A.ulam","element A { Bool(3) a, b, c; Unary(3) d; use test;  a = true; b = false; c = a + b; d = c; return c; } }");
      bool rtn2 = fms->add("test.ulam", "Int test() {");
      
      if(rtn1 & rtn2)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3158_test_compiler_booladd)
  
} //end MFM


