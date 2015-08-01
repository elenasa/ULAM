#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3485_test_compiler_voidfuncreturnscastedatom)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Tu { Bool(3) me(false);  Int(32) test() {  ( )func 0 cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nelement Tu {\n Bool(3) me;\nVoid func() {\nAtom a;\n return (Void) a;\n}\n Int test(){\n func();\nreturn 0;\n}\n}\n");

      if(rtn1)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3485_test_compiler_voidfuncreturnscastedatom)

} //end MFM
