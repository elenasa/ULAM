#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3126_test_compiler_typedef_scope)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { Int(32) x(4);  Int(32) test() {  typedef Int(32) Bar[2];  Int(32) e[2];  { e 0 [] 4 = } x e 0 [] = x return } }\nExit status: 4");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { Int x; Int test() { typedef Int Bar[2]; Bar e; { e[0] = 4; } /* match int return type */ x= e[0]; return x; } }");  
      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3126_test_compiler_typedef_scope)
  
} //end MFM


