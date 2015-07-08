#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3229_test_compiler_quarkself_MP_error)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_T { S(3u) m_s( constant Unsigned(32) a = 3; );  Bool(3) b1(true);  Bool(3) b2(true);  Int(32) test() {  S(1u) s;  S(1u) t;  b1 m_s iep(7u) . = b2 t iep(1u) . cast = s iep(1u) . cast return } }\nUq_S { constant Unsigned(32) a = NONREADYCONST;  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3228
      bool rtn2 = fms->add("T.ulam"," ulam 1;\n use S;\n element T{\nS(3) m_s;\nBool(3) b1,b2;\n Int test() {\nS(1) s, t;\n b1 = m_s.iep;\n b2 = t.iep;\n return s.iep;\n }\n }\n");

      bool rtn1 = fms->add("S.ulam"," ulam 1;\n quark S(Unsigned a){\nparameter Bool(a) iep = (a != 0);\n  }\n"); //self.iep fails


      if(rtn1 && rtn2)
	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3229_test_compiler_quarkself_MP_error)

} //end MFM
