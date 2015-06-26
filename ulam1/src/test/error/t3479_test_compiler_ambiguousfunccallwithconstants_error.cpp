#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3479_test_compiler_ambiguousfunccallwithconstants_error)
  {
    std::string GetAnswerKey()
    {
      //no longer takes the first one even though constant fits both funcs!
      //./Tu.ulam:12:9: ERROR: Ambiguous matches (2) of function <func> with 1 matching argument type; Explicit casting required.
      return std::string("Exit status: -1\nUe_Tu { typedef Int(3) I;  typedef Int(4) J;  Int(32) test() {  ( 3 cast )func cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //t3476 uses 9; here 3 fits both.
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\ntypedef Int(3) I;\n I func(I arg) {\nreturn arg;\n}\n typedef Int(4) J;\n J func(J arg) {\nreturn arg;\n}\n Int test(){\n return func(3);\n}\n}\n");


      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3479_test_compiler_ambiguousfunccallwithconstants_error)

} //end MFM
