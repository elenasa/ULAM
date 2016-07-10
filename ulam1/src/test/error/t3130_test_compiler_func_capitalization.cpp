#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3130_test_compiler_func_capitalization)
  {
    std::string GetAnswerKey()
    {
      //./D.ulam:1:24: ERROR: Name of variable/function <Times>: Identifier must begin with a lower-case letter.
      return std::string("Ue_D { Int t(15);  Int test() {  t ( 4 5 )Times = } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","element D { Int t; Int Times(Int m, Int n) { Int e; while( m-=1 ) e += n; return e; } Int test(){\n{\nt = Times(4,5); return t; } } }");

      if(rtn1)
	return std::string("D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3130_test_compiler_func_capitalization)

} //end MFM
