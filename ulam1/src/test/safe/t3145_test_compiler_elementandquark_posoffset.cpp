#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3145_test_compiler_elementandquark_posoffset)
  {
    std::string GetAnswerKey()
    {
      // rearranged to avoid Word Boundary constraints
      //note: order of appearance of 'Bool b' first; not in parse-tree order to print their values.
      return std::string("Exit status: 7\nUe_Foo { Bool(1) b(false);  typedef Bar Pop[2];  Bool(1) a(false);  Bar m_bar2[2]( Bool(1) val_b[3](false,false,false);  Bool(1) val_b[3](false,false,false); );  Int(32) m_i(7);  Bar m_bar1( Bool(1) val_b[3](false,false,false); );  Bar m_bar3( Bool(1) val_b[3](false,false,false); );  Int(32) test() {  Foo f;  f ( 1 )check . m_i 7 = m_i return } }\nUq_Bar { Bool(1) val_b[3](false,false,false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      /* (without word boundary restriction) in Ud_0024613Foo.h:
	 Ut_001114Bool Um_11a;
	 Uq_001313Bar<1> Um_16m_bar1;
	 Ut_0023213Int Um_13m_i;
	 Ut_121313Bar<36> Um_16m_bar2;
	 Ut_001114Bool Um_11b;
	 Uq_001313Bar<43> Um_16m_bar3;
      */

      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n use Bar;\n element Foo {\n typedef Bar Pop[2];\n Bool a;\nPop m_bar2;\n Int m_i;\nBool b;\n Bar m_bar1;\nBar m_bar3;\n Bool check(Int v) {\n return true;\n }\n Int test() {\n Foo f;\n f.check(1);\n m_i = 7;\n return m_i;\n }\n }\n"); //tests offsets, but too complicated data members in Foo for memberselect

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool val_b[3];\n  Void(0) reset(Bool b) {\n b = 0;\n return;\n}\n }\n");

      if(rtn1 && rtn2)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3145_test_compiler_elementandquark_posoffset)

} //end MFM
