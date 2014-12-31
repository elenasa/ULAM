#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3190_test_compiler_quark_array2d_quarkcasttoint)
  {
    std::string GetAnswerKey()
    {
      return std::string("Uq_C2D { Int(8) m_width(0);  Int(8) m_height(0);  Int(8) m_x(0);  Int(8) m_y(0);  <NOMAIN> }\nExit status: -1");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("C2D.ulam","quark C2D {\n Int(8) m_width, m_height, m_x, m_y;\n  Void init(Int x, Int y) {\n m_width = x;\n m_height = y;\n}\n Void init() {\n m_width = 9;\n m_height = 4;\n /* event window overload */ }\n Void set(Int a, Int b) {\n m_x = a;\n m_y = b;\n }\n Int toInt(){\n return ((m_height-m_y) * m_width + (m_height-m_x));\n }\n }\n");

      if(rtn2)
	return std::string("C2D.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3190_test_compiler_quark_array2d_quarkcasttoint)

} //end MFM


