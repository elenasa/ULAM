#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3229_test_compiler_quarkself_MP)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_S { Bool(3) b1(false);  Bool(3) b2(false);  Int(32) test() {  self iep(7u) . cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3259 only quark has the MP!
      bool rtn2 = fms->add("T.ulam"," ulam 1;\n use S;\n element T{\n Int test() {\nS s;\n return /*s.iep */ 0;\n }\n }\n");

      //./S.ulam:6:14: ERROR: Member selected must be either a quark or an element, not type: Atom(96); suggest using a Conditional-As.
      bool rtn1 = fms->add("S.ulam"," ulam 1;\n quark S{\nBool(3) b1, b2;\n parameter Bool(3) iep = true;\n Bool(3) getmp() {\n return iep;\n }\n }\n"); //self.iep fails


      if(rtn1 && rtn2)
	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3229_test_compiler_quarkself_MP)

} //end MFM
