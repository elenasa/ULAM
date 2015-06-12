#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3229_test_compiler_quarkselfMP_error)
  {
    std::string GetAnswerKey()
    {
      // gcc complains:
      /*
	include/Uq_10161S10.h:66:26: error: invalid use of member ‘MFM::Uq_10161S10<EC, POS>::Up_3iep’ in static member function
	mutable Ui_Ut_10131b Up_3iep;                          ^
      */
      //./S.ulam:4:12: ERROR: Model Parameters can survive in elements only.
      return std::string("Exit status: 0\nUe_T { Int(32) test() {  S s;  0 return } }\nUq_S { Bool(3) b1(false);  Bool(3) b2(false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3259 only quark has the MP!
      bool rtn2 = fms->add("T.ulam"," ulam 1;\n use S;\n element T{\n Int test() {\nS s;\n return s.iep;\n }\n }\n");

      //self.iep fails:
      //./S.ulam:6:14: ERROR: Member selected must be either a quark or an element, not type: Atom(96); suggest using a Conditional-As.
      bool rtn1 = fms->add("S.ulam"," ulam 1;\n quark S{\nBool(3) b1, b2;\n parameter Bool(3) iep = true;\n /*Bool(3) getmp() {\n return iep;\n }\n*/ }\n"); //return self.iep fails


      if(rtn1 && rtn2)
	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3229_test_compiler_quarkselfMP_error)

} //end MFM
