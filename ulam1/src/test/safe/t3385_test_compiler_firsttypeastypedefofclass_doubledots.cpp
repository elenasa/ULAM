#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3385_test_compiler_firsttypeastypedefofclass_doubledots)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: -4\nUe_F { Int(32) test() {  -4 cast return } }\nUe_A { typedef X H;  typedef Int(3) ZZ;  <NOMAIN> }\nUq_X { typedef B Y;  <NOMAIN> }\nUe_B { typedef Int(3) Z;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // B before X too..(informed by t3380)
      bool rtn1 = fms->add("F.ulam","use A;\n use B;\n use X;\n element F{ Int test() {\n return A.ZZ.minof;\n}\n }\n");

      bool rtn2 = fms->add("A.ulam", "element A{\ntypedef X H;\n typedef H.Y.Z ZZ;\n}\n");

      bool rtn3 = fms->add("X.ulam", "quark X{\n typedef B Y;\n }\n");

      bool rtn4 = fms->add("B.ulam", "element B{\n typedef Int(3) Z;\n }\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("F.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3385_test_compiler_firsttypeastypedefofclass_doubledots)

} //end MFM
