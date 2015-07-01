#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3197_test_compiler_elementandquark_usercastquarknotoint_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:1:152: ERROR: (2) <toInt> is not a defined function, and cannot be called.
      return std::string("Exit status: 4\nUe_Foo { Unary(4) m_i(4);  Bar m_bar( Bool(1) val_b[3](false,true,false); );  Int(32) test() {  m_bar ( 1 )check = m_i m_bar ( )toInt . cast cast = m_i cast return } }\nUq_Bar { Bool(1) val_b[3](true,true,true);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Bar;\n element Foo {\n Unary(4) m_i;\n Bar m_bar;\n Bar check(Int v) {\n Bar b;\n b.val_b[1] = true;\n return b;\n } Int test() {\n m_bar = check(1);\n m_i = (Unary(4)) m_bar;\n return m_i;\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = false;\n }\n /*Int toInt(){\n return 7;\n}*/\n }\n");

      if(rtn1 & rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3197_test_compiler_elementandquark_usercastquarknotoint_error)

} //end MFM
