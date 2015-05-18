#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3129_test_compiler_funcdefshadow)
  {
    std::string GetAnswerKey()
    {
      //./D.ulam:3:5: ERROR: Undefined function <times> that has already been declared as a variable.
      return std::string("Ue_D { Int(32) t(15);  Int(32) test() {  t ( 4 5 )times = } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","element D { Int t; Int times(Int m, Int n) { Int e; while( m-=1 ) e += n; return e; } Int test(){\n{Int times;\nt = times(4,5); return t; } } }");

      if(rtn1)
	return std::string("D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3129_test_compiler_funcdefshadow)

} //end MFM
