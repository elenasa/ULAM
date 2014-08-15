#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3125_test_compiler_typedef_scope)
  {
    std::string GetAnswerKey()
    {
      return std::string(" { Int test() {  { typedef Int Bar[2];  Bar e[2];  e 0 [] 4 = } 3 } }\n");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // Bar f gave error: a.ulam:1:69: (NodeTerminalIdent.cpp:labelType:61) ERROR: (2) <f> is not defined, and cannot be called.
      bool rtn1 = fms->add("a.ulam","ulam { Int test() { { typedef Int Bar[2]; Bar e; e[0] = 4; }  Bar f; f[0] = 3;  /* match return type */ return f[0]; } }");  
      
      if(rtn1)
	return std::string("a.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3125_test_compiler_typedef_scope)
  
} //end MFM


