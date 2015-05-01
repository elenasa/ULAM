#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3382_test_compiler_twoarrayswithsameunknownkeys_issue)
  {
    std::string GetAnswerKey()
    {

      //test bug fix: incrementUnknown in setUTIsizes (CS)
      //test: CompilerState.cpp:702: MFM::UlamType* MFM::CompilerState::getUlamTypeByIndex(MFM::u16): Assertion `isDefined(m_indexToUlamKey[typidx], rtnUT)' failed. Aborted (core dumped)
      return std::string("Exit status: 0\nUq_R { /* empty class block */ }Ue_Palette { typedef Unsigned(6) A[4];  typedef Unsigned(6) B[4];  Int(32) test() {  Unsigned(6) color[4];  Unsigned(6) gray[4];  0 return } }\nUq_Color { typedef Unsigned(6) A[4];  typedef Unsigned(6) B[4];  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn3 = fms->add("R.ulam", "use Palette;\n use Color;\n quark R{\n }\n");

      bool rtn1 = fms->add("Palette.ulam","element Palette{\ntypedef Color.A A;\n typedef Color.B B;\n Int test() {\nA color;\n B gray;\n return 0;\n }\n }");
      bool rtn2 = fms->add("Color.ulam","ulam 1;\n quark Color{\ntypedef Unsigned(6) A[4];\n typedef Unsigned(6) B[4];\n  }");


      // simplify for debug..
      //bool rtn1 = fms->add("Palette.ulam","element Palette{\nColor.A color;\n Int test() {\n return 0;\n }\n }");
      //bool rtn2 = fms->add("Color.ulam","quark Color{\ntypedef Unsigned(6) A[4];\n }");

      if(rtn1 && rtn2 && rtn3)
	return std::string("R.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3382_test_compiler_twoarrayswithsameunknownkeys_issue)

} //end MFM
