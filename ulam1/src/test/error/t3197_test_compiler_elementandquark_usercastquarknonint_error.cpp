#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3197_test_compiler_elementandquark_usercastquarknonint_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:1:152: ERROR: Cannot cast quark type: Bar(UNKNOWN)<11> to non-Int type: Unary(32).
      //./Foo.ulam:1:152: ERROR: Cannot cast quark type: Bar(3)<11> to non-Int type: Unary(32).
      return std::string("Ue_Foo {  } }\nExit status: 7");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo { Unary(4) m_i; Bar m_bar; Bar check(Int v) { Bar b; b.val_b[1] = true; return b; } Int test() { m_bar = check(1); m_i = (Unary) m_bar; return m_i; } }\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1; quark Bar { Bool val_b[3];  Void reset(Bool b) { b = 0; } Int toInt(){ return 7;} }\n");

      if(rtn1 & rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3197_test_compiler_elementandquark_usercastquarknonint_error)

} //end MFM
