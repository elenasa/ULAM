#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3136_test_compiler_twofuncdefs)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int x(15);  Int y(0);  Int main() {  y x ( 4 5 )times = = y ( x x )max = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int times(Int m, Int n) { Int e; while( m-=1 ) e += n; e; } Int max(Int a, Int b) { a - b; } Int x, y; Int main(){ y = x = times(4,5); y = max(x,x); } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3136_test_compiler_twofuncdefs)
  
} //end MFM


