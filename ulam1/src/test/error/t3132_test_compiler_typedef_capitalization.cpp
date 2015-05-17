#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3132_test_compiler_typedef_capitalization)
  {
    std::string GetAnswerKey()
    {
      //./D.ulam:1:26: ERROR: Invalid typedef Alias <too>, Type Identifier (2nd arg) requires capitalization.
      return std::string("Uq_D { Int(4) t(15);  Int(32) test() {  t ( 4 5 )times = } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","quark D { typedef Int(4) too; too times(Int(4) m, Int(4) n) { Int(4) e; while( m-=1 ) e += n; e; } Int test(){\n{\nt = times(4,5); return t; } } }");

      if(rtn1)
	return std::string("D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3132_test_compiler_typedef_capitalization)

} //end MFM
