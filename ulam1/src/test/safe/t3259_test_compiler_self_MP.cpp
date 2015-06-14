#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3259_test_compiler_self_MP)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_S { Bool(3) b1(false);  Bool(3) b2(false);  Int(32) test() {  self iep(7u) . cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //eval ans doesn't do as.
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S{\nBool sp;\n Bool(3) b1, b2;\n element Bool(3) iep;\n Int test() {\nAtom a, t;\nself.iep = true;\n a = self;\n if(a as S) b2 = a.iep;\n return b2;\n }\n }\n");

      bool rtn1 = fms->add("S.ulam"," ulam 1;\n element S{\nBool(3) b1, b2;\n parameter Bool(3) iep = true;\n Int test() {\n return self.iep;\n }\n }\n");



      if(rtn1)
	return std::string("S.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3259_test_compiler_self_MP)

} //end MFM
