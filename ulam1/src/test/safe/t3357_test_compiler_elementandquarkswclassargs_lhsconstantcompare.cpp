#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3357_test_compiler_elementandquarkswclassargs_lhsconstantcompare)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_Foo { Int(32) test() {  Booly(true) b;  b ( )behave . cast return } }\nUq_Booly { constant Bool(1) bomb = NONREADYCONST;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by issue: also tests Bool as class parameter
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Booly;\nelement Foo{\nInt test(){\nBooly(true) b;\n return (Int) b.behave();\n}\n}\n");

      bool rtn2 = fms->add("Booly.ulam","quark Booly(Bool bomb) {\nBool behave(){\n if(bomb != false) return true;\nreturn false;\n}\n}\n");

      if(rtn1 && rtn2 )
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3357_test_compiler_elementandquarkswclassargs_lhsconstantcompare)

} //end MFM
