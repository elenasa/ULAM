#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3483_test_compiler_elementandquarkunseen_noparamswargs_error)
  {
    std::string GetAnswerKey()
    {
      //./NoParms.ulam:2:7: ERROR: Conflicting class args previously seen for class with no parameters <NoParms>
      //Unrecoverable Program Parse FAILURE.
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
      //informed by 3340, except seeing quark afterwards
      bool rtn1 = fms->add("NoParms.ulam", "ulam 1;\nquark NoParms {\n }\n");

      //bool rtn2 = fms->add("Foo.ulam", "ulam 1;\nuse NoParms;\nelement Foo {\n NoParms(1) boom;\n}\n");
      bool rtn2 = fms->add("Foo.ulam", "ulam 1;\nelement Foo {\n NoParms(1) boom;\n}\nuse NoParms;\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3483_test_compiler_elementandquarkunseen_noparamswargs_error)

} //end MFM
