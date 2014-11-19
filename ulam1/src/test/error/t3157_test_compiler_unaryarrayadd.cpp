#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3157_test_compiler_unaryarrayadd)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { typedef Unary(3) Mu[2];  Unary(3) a[2](2,1);  Unary(3) b[2](3,1);  Unary(3) c[2](3,2);  Int(32) test() {  a 0 [] 2 cast = a 1 [] 1 cast = b 0 [] 3 cast = b 1 [] 1 cast = c a b +b = c 0 [] cast return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      //exercises, append in NodeBinaryOpAdd; 
      //note2: c[0] total exceeds 3 bits, so we get the max.
      bool rtn1 = fms->add("A.ulam","element A { typedef Unary(3) Mu[2]; Mu a, b, c; use test;  a[0] = 2; a[1] = 1; b[0] = 3; b[1] = 1; c = a + b; return c[0]; } }");
      bool rtn2 = fms->add("test.ulam", "Int test() {");
      
      if(rtn1 & rtn2)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3157_test_compiler_unaryarrayadd)
  
} //end MFM


