#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3128_test_compiler_decllist_witharray)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(3);  Int c[2](0,3);  Int b(2);  Int test() {  a 1 = b 2 = a b += c 1 [] a = a return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int a, c[2], b; use test;  a = 1; b = 2; a+=b; c[1] = a; return a;} }");
      bool rtn2 = fms->add("test.ulam", "Int test() {");
      
      if(rtn1 & rtn2)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3128_test_compiler_decllist_witharray)
  
} //end MFM


