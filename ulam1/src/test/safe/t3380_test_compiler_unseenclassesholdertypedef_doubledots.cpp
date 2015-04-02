#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3380_test_compiler_unseenclassesholdertypedef_doubledots)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 0\nUe_F { Int(32) test() {  0 return } }\nUe_A { typedef Int(32) ZZ;  <NOMAIN> }\nUq_X { typedef B Y;  <NOMAIN> }\nUe_B { typedef Int(32) Z;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // try B before X too..t3381
      bool rtn1 = fms->add("F.ulam","use A;\n use X;\n use B;\n element F{ Int test() {\n return 0;\n}\n }\n");
      //bool rtn1 = fms->add("F.ulam","use A;\n use B;\n use X;\n element F{ Int test() {\n return 0;\n}\n }\n");

      bool rtn2 = fms->add("A.ulam", "element A{\n typedef X.Y.Z ZZ;\n}\n");

      bool rtn3 = fms->add("X.ulam", "quark X{\n typedef B Y;\n }\n");

      bool rtn4 = fms->add("B.ulam", "element B{\n typedef Int Z;\n }\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("F.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3380_test_compiler_unseenclassesholdertypedef_doubledots)

} //end MFM
