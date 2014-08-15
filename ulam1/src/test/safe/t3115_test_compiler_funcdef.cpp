#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3115_test_compiler_funcdef)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int a(4);  Int b(5);  Int d(15);  Int test() {  a 4 = b 5 = d ( a b )times = d return } }\nExit status: 15");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int times(Int m, Int n) { Int d; while( m-=1 ) d += n; return d; } Int a, b, d; Int test(){ a=4; b=5; d = times(a,b); return d; } }");
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3115_test_compiler_funcdef)
  
} //end MFM


