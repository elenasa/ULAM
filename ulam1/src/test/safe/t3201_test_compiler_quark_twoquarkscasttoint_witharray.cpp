#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3201_test_compiler_quark_twoquarkscasttoint_witharray)
  {
    std::string GetAnswerKey()
    {
      //developed by Dave (10092014) in c++ and then converted to Ulam.
      // 7 is max that Int(4) can be positively, so 12 becomes 7.
      return std::string("Uq_Bar { Bool(1) b(false);  Unsigned(3) x(0);  Unsigned(3) y(0);  <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //note: don't have <<2, so substituted *4
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nBool b;\nUnsigned(3) x, y;\nInt toInt(){\nreturn (x * 4) + y;\n}\n}\n");

      if(rtn2)
	return std::string("Bar.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3201_test_compiler_quark_twoquarkscasttoint_witharray)

} //end MFM


