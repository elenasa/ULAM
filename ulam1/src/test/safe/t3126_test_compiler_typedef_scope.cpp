#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3126_test_compiler_typedef_scope)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int x(4);  Int main() {  typedef Int Bar[2];  Int e[2];  { e 0 [] 4 = } x e 0 [] = } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("a.ulam","ulam { Int x; Int main() { typedef Int Bar[2]; Bar e; { e[0] = 4; } /* match int return type */ x= e[0]; } }");  
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3126_test_compiler_typedef_scope)
  
} //end MFM


