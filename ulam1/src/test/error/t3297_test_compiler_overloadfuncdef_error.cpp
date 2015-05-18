#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3297_test_compiler_overloadfuncdef_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./Tu.ulam:8:6: ERROR: Check overloading function: <chk> has a duplicate definition: (Uf_3chkUt_10131i), while compiling class: Tu.
      */
      return std::string("Ue_Tu { }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //no matching function: error should list the type, not just the number
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\n element Tu {\nBool(3) sp;\n Int chk(Int(Tu.sizeof) i){\n return 0;\n}\n typedef Int(3) Poo;\n Int chk(Poo p){\n return 1;\n}\n Int test(){\n return 0;\n}\n}\n");

      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3297_test_compiler_overloadfuncdef_error)

} //end MFM
