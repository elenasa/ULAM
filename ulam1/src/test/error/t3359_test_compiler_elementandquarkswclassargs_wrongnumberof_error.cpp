#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3359_test_compiler_elementandquarkswclassargs_wrongnumberof_error)
  {
    std::string GetAnswerKey()
    {
      // ERROR: number of arguments (2) in class instance: Booly, does not match the required number of parameters (3).
      return std::string("Exit status: -1\nUe_Foo { Int(32) test() {  Booly(firstRange,lastRange,bomb) b;  b ( )behave . cast return } }\nUq_Booly { constant Unsigned(3) firstRange = NONREADYCONST;  constant Unsigned(3) lastRange = NONREADYCONST;  constant Bool(1) bomb = NONREADYCONST;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by issue: missing Bool as 3rd class parameter in another class
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Booly;\nelement Foo{\nInt test(){\nBooly(0u, 2u) b;\n return b.behave();\n}\n}\n");

      bool rtn2 = fms->add("Booly.ulam","quark Booly(Unsigned(3) firstRange, Unsigned(3) lastRange, Bool bomb) {\nBool behave(){\n if(bomb && lastRange != firstRange) return true;\nreturn false;\n}\n}\n");

      if(rtn1 && rtn2 )
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3359_test_compiler_elementandquarkswclassargs_wrongnumberof_error)

} //end MFM
