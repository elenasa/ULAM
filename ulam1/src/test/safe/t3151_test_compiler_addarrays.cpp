#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3151_test_compiler_addarrays)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_A { typedef Int Foo[2];  Int d[2](7,5);  Int test() {  Int f[2];  f 0 [] 1 = f 1 [] 2 = Int e[2];  e 0 [] 6 = e 1 [] 3 = d e f +b = d 0 [] return } }\nExit status: 0");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A { typedef Int Foo [2]; Int test() { Foo f; f[0] = 1; f[1] = 2; Foo e; e[0] = 6; e[1] = 3; d = e + f; return d[0]; /* match return type */}\nFoo d; }");  // want d[0] == 7. NOTE: requires 'operator+=' in c-code to add arrays

      
      if(rtn1)
	return std::string("A.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3151_test_compiler_addarrays)
  
} //end MFM


