#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3360_test_compiler_elementandquarkswclassargs_memberconstantasstatement)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 2\nUe_Foo { Int(32) test() {  Booly(0u,2u,true) b;  b true . b 2u . cast return } }\nUq_Booly { constant Unsigned(3) firstRange = NONREADYCONST;  constant Unsigned(3) lastRange = NONREADYCONST;  constant Bool(1) bomb = NONREADYCONST;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by issue (see 3359): also tests Bool as 3rd class parameter
      // b.bomb has NodeMemberSelect as parent, not NodeStatements event though its a "solo" statement
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Booly;\nelement Foo{\nInt test(){\nBooly(0u, 2u, true) b;\nb.bomb;\n return b.lastRange;\n}\n}\n");

      bool rtn2 = fms->add("Booly.ulam","quark Booly(Unsigned(3) firstRange, Unsigned(3) lastRange, Bool bomb) {\nBool behave(){\n if(bomb && lastRange != firstRange) return true;\nreturn false;\n}\n}\n");

      if(rtn1 && rtn2 )
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3360_test_compiler_elementandquarkswclassargs_memberconstantasstatement)

} //end MFM
