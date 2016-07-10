#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3469_test_compiler_element_bitstypearith_error)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:11:18: ERROR: Incompatible Bits type for binary operator+. Suggest casting to an ordered type first.
      return std::string("Exit status: -1\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3194
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\n element Foo {\nBits(3) m_i[3];\n Int test() {\n m_i[0] = 1;\n m_i[1] = 2;\n m_i[2] = m_i[0] + m_i[1];\nreturn m_i[2];\n }\n }\n");

      if(rtn1)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3469_test_compiler_element_bitstypearith_error)

} //end MFM
