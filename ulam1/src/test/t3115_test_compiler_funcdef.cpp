#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3115_test_compiler_funcdef)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) d(15);  Int(32) test() {  d ( 4 cast 5 cast )times = d return } }\nExit status: 15");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int times(Int m, Int n) { Int d; while( m-=1 ) d += n; return d; } Int d; Int test(){ d = times(4,5); return d; } }");
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3115_test_compiler_funcdef)
  
} //end MFM


