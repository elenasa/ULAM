#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3128_test_compiler_decllist_witharray)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) a(3);  Int(32) test() {  Int(32) b;  Int(32) c[2];  b 1 = b 2 += c 1 [] b = a c 1 [] = b return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int a; use test;  b = 1; b+=2; c[1] = b; a = c[1]; return b;} }");
      bool rtn2 = fms->add("test.ulam", "Int test() { Int b, c[2]; ");
      
      if(rtn1 & rtn2)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3128_test_compiler_decllist_witharray)
  
} //end MFM


