#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3218_test_compiler_element_modelparameteratom_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:8:12: ERROR: Only primitive types may be a Model Parameter, not: <Atom>.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Poo;\nelement Foo {\nBool(1) sp;\n parameter Poo poochance;\n Bool last;\n Int test() {\n return -1;\n }\n }\n");

      bool rtn2 = fms->add("Poo.ulam"," ulam 1;\n element Poo {\n Bool(3) valb[3];\n  Void reset(Bool b) {\n valb[1] = b;\n }\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3218_test_compiler_element_modelparameteratom_error)

} //end MFM
