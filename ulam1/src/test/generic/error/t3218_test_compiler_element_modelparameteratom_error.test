#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3218_test_compiler_element_modelparameteratom_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:5:12: ERROR: Invalid Model Parameter Type: <Atom>: Only primitive types beginning with an upper-case letter may be a Model Parameter.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nelement Foo {\nBool(1) sp;\n parameter Atom poochance;\n Bool last;\n Int test() {\n return -1;\n }\n }\n");

      if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3218_test_compiler_element_modelparameteratom_error)

} //end MFM
