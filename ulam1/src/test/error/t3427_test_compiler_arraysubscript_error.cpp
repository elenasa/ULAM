#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3427_test_compiler_arraysubscript_error)
  {
    std::string GetAnswerKey()
    {
      //./A.ulam:6:6: ERROR: Array subscript [1] exceeds the size (1) of array 'site'.
      return std::string("Exit status: -1\nUe_A { typedef Int(32) BigSite[1];  Int(32) test() {  Int(32) site[10];  { Int(32) i;  i 0 = i 10 < cond { site i [] i = } _1: i 1 += while } 32 return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","element A {\n typedef Int(32) BigSite[1];\nInt test(){ BigSite site; for(Int i = 0; i < 10; ++i){\n site[i] = i;\n }\n return site.sizeof;\n }\n }\n");

      if(rtn1)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3427_test_compiler_arraysubscript_error)

} //end MFM
