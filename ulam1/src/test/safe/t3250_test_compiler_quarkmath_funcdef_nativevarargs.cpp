#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3250_test_compiler_quarkmath_funcdef_nativevarargs)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_Math { <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // maxof with variable number of args...
      //with final void * 0 instead of nargs as first arg.
      bool rtn2 = fms->add("Math.ulam", "ulam 1;\nquark Math {\nInt max(...) native;\n}\n");

      if(rtn2)
	return std::string("Math.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3250_test_compiler_quarkmath_funcdef_nativevarargs)

} //end MFM


