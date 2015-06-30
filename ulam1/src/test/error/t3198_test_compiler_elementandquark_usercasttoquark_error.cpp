#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3198_test_compiler_elementandquark_usercasttoquark_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:8:11: ERROR: Cannot cast Bool(3) to type: Bar.
      return std::string("Ue_Foo { Int(4) m_i(7);  Bar m_bar( Bool(1) val_b[3](false,true,false); );  Int(32) test() {  m_bar ( 1 cast )check = m_i m_bar ( )toInt . cast = m_i cast return } }\nExit status: 7");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Bar;\n element Foo {\n Bool(3) m_b;\n Bar m_bar;\n Int test() {\n m_b = true;\n m_bar = (Bar) m_b;\n return m_b;\n }\n }\n");

      // and if we do..
      //./Foo.ulam:9:14: ERROR: RHS of operator <as> is an array: TYPE_IDENT, operation deleted.
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Bar;\n element Foo {\n typedef Bool(1) Barr[3];\n Barr m_b;\n Bar m_bar;\n Int test() {\n m_b[0] = true;\n if(m_bar as Barr) {\n m_bar = m_b;\n }\n return m_c;\n }\n }\n");

      //different test..needs new number.
      //./Foo.ulam:4:4: ERROR: Cannot CAST type: Bar(3)<11> as a Atom(96).
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo {\n Bar m_bar;\n Int test() { Atom a;\n a = m_bar;\n return 0;\n } }\n");
      //eval returns wrong answer! but compiles
      //bool rtn1 = fms->add("Foo.ulam","ulam 1; use Bar; element Foo {\n Bar m_bar;\n Int test() { Atom a;\n a = self;\n if(a has Bar) {\n return 1;\n}\n return 0;\n } }\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void reset(Bool b) {\n b = false;\n }\n Int toInt(){\n return 7;\n}\n }\n");

      if(rtn1 & rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3198_test_compiler_elementandquark_usercasttoquark_error)

} //end MFM
