#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3486_test_compiler_usefilemissing_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./Foo.ulam:5:1: ERROR: Unexpected ABORT ERROR Token: Cannot fufill 'use NoParms.ulam' request.
	Unrecoverable Program Parse FAILURE.
       */
      return std::string("Ue_Foo { NoParms boom();  <NOMAIN> }\nUq_NoParms { /* empty class block */ }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      /*
	ulam 1;
	quark NoParms {
	}
	element Foo {
	NoParms(1) boom;
	}
      */

      // missing use file.
      //informed by 3340, except seeing quark afterwards
      //bool rtn1 = fms->add("NoParms.ulam", "ulam 1;\nquark NoParms {\n }\n");
      bool rtn2 = fms->add("Foo.ulam", "ulam 1;\nelement Foo {\n NoParms(1) boom;\n}\nuse NoParms;\n");

      if(rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3486_test_compiler_usefilemissing_error)

} //end MFM
