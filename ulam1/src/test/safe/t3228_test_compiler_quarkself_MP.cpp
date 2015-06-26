#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3228_test_compiler_quarkself_MP)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_T { S m_s( Bool(3) b1(false);  Bool(3) b2(false); );  Int(32) test() {  S s;  s iep(7u) . cast return } }\nUq_S { Bool(3) b1(false);  Bool(3) b2(false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3259 only quark has the MP!
      bool rtn2 = fms->add("T.ulam"," ulam 1;\n use S;\n element T{\nS m_s;\n Int test() {\nS s;\n return s.iep /*s.getmp()*/;\n }\n }\n");

      bool rtn1 = fms->add("S.ulam"," ulam 1;\n quark S{\nBool(3) b1, b2;\n parameter Bool(3) iep = true;\n Bool(3) getmp() {\n return iep;\n }\n }\n"); //self.iep fails because it's an atom.


      if(rtn1 && rtn2)
	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3228_test_compiler_quarkself_MP)

} //end MFM
