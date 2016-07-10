#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3232_test_compiler_expressionwithparens_issue)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 72\nUe_Foo { typedef Unsigned(8) Channel;  typedef Unsigned(8) ARGB[4];  Unsigned(3) m_lastTouch(2);  Int(32) test() {  Unsigned(8) tmp[4];  m_lastTouch 2 cast = tmp 1 [] m_lastTouch cast 36u cast * cast = tmp 1 [] cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Foo.ulam", "ulam 1;\n element Foo {\ntypedef Unsigned(8) Channel;\n typedef Channel ARGB[4]; Unsigned(3) m_lastTouch;\n Int test(){\nARGB tmp;\n m_lastTouch = 2;\n tmp[1] = (Channel) (m_lastTouch * (Channel.maxof / m_lastTouch.maxof));\n return tmp[1];\n }\n }\n");

      if(rtn2)
      return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3232_test_compiler_expressionwithparens_issue)

} //end MFM
