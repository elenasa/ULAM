#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3136_test_compiler_twofuncdefs)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) x(15);  Int(32) y(0);  Int(32) test() {  y x ( 4 cast 5 cast )times = = y ( x x )max = y return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int times(Int m, Int n) { Int e; while( m-=1 ) e += n; return e; } Int max(Int a, Int b) { return (a - b); } Int x, y; Int test(){ y = x = times(4,5); y = max(x,x); return y; } }");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3136_test_compiler_twofuncdefs)
  
} //end MFM


