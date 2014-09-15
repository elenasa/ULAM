#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3150_test_compiler_plusequalarray)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { typedef Int Foo[2];  Int d[2](7,3);  Int test() {  d 0 [] 1 = Int e[2];  e 0 [] 6 = e 1 [] 3 = d e += d 0 [] return } }\nExit status: 7");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { typedef Int Foo [2]; Int test() { d[0] = 1; Foo e; e[0] = 6; e[1] = 3; d += e; return d[0]; /* match return type */}\nFoo d; }");  // want d[0] == 7. NOTE: requires 'operator+=' in c-code to add arrays

      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3150_test_compiler_plusequalarray)
  
} //end MFM


