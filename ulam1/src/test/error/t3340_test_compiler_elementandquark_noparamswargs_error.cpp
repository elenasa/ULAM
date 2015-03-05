#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3340_test_compiler_elementandquark_noparamswargs_error)
  {
    std::string GetAnswerKey()
    {
      //Foo.ulam:3:13: ERROR: Class without parameters already exists with the same name: NoParms <UTI10>.
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

      bool rtn1 = fms->add("NoParms.ulam", "ulam 1;\nquark NoParms {\n }\n");

      bool rtn2 = fms->add("Foo.ulam", "ulam 1;\nuse NoParms;\nelement Foo {\n NoParms(1) boom;\n}\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3340_test_compiler_elementandquark_noparamswargs_error)

} //end MFM
