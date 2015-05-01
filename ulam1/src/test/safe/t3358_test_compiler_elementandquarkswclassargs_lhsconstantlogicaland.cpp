#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3358_test_compiler_elementandquarkswclassargs_lhsconstantlogicaland)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_Foo { Int(32) test() {  Booly(7u,8u,true) b;  b ( )behave . cast return } }\nUq_Booly { constant Unsigned(3) firstRange = NONREADYCONST;  constant Unsigned(3) lastRange = NONREADYCONST;  constant Bool(1) bomb = NONREADYCONST;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by issue (see 3359): also tests Bool as 3rd class parameter
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Booly;\nelement Foo{\nInt test(){\nBooly(0u, 2u, true) b;\n return b.behave();\n}\n}\n");

      // this version also tests class arguments with bitsize, and saturation (i.e. 8 == 7)
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Booly;\nelement Foo{\nInt test(){\nBooly(7u, 8u, true) b;\n return b.behave();\n}\n}\n");

      bool rtn2 = fms->add("Booly.ulam","quark Booly(Unsigned(3) firstRange, Unsigned(3) lastRange, Bool bomb) {\nBool behave(){\n if(bomb && lastRange != firstRange) return true;\nreturn false;\n}\n}\n");

      if(rtn1 && rtn2 )
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3358_test_compiler_elementandquarkswclassargs_lhsconstantlogicaland)

} //end MFM
