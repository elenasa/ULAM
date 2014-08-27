#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3136_test_compiler_twofuncdefs)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int x(15);  Int y(0);  Int test() {  y x ( 4 5 )times = = y ( x x )max = y return } }\nExit status: 0");
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


