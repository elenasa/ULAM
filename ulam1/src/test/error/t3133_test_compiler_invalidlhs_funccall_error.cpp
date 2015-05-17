#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3133_test_compiler_invalidlhs_funccall_error)
  {
    std::string GetAnswerKey()
    {
      //./D.ulam:1:67: ERROR: Lefthand side of equals is 'Not StoreIntoAble': <foo>, type: Int(8).
      return std::string(" \n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("D.ulam","quark D { Int(8) a; Int(8) foo() { return 1; } Int test() { foo() = a; return foo(); } }");

      if(rtn1)
	return std::string("D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3133_test_compiler_invalidlhs_funccall_error)

} //end MFM
