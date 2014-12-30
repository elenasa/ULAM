#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3187_test_compiler_quark_twoargscast)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_C2D { Int(6) m_width(0);  Int(6) m_height(0);  <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("C2D.ulam","quark C2D {\n Int(6) m_width;\n Int(6) m_height;\n  Void init(Int x, Int y) {\n m_width = x;\n m_height = y;\n return;\n }\n Void init() {\n m_width = 9;\n m_height = 4;\n return;\n /* event window overload */ }\n Int getIndex(Int a, Int b){\n return ((m_height-b) * m_width + (m_height-a));\n }\n }\n");

      if(rtn2)
	return std::string("C2D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3187_test_compiler_quark_twoargscast)

} //end MFM


