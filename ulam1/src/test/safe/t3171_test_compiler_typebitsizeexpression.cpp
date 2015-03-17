#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3171_test_compiler_typebitsizeexpression)
  {
    std::string GetAnswerKey()
    {
      //m_i[1] = max == 3, instead of lower 3 bits of 8 (==0).

      return std::string("Exit status: 8\nUe_C { Bool(1) a(false);  Int(3) m_i[4](3,3,0,0);  Int(6) m_u(8);  Bool(3) b(false);  Int(32) test() {  Int(32) a;  a 3 = m_i 0 [] a cast = Int(5) exp;  exp 8 cast = m_u exp cast = m_i 1 [] exp cast = exp cast return } }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // doesn't work with variables, local or data members: Int(a), Int(2+a), OR Int(2+m_i[0]);
      bool rtn1 = fms->add("C.ulam","ulam 1;\n element C {\n Bool a;\n Int(3) m_i[4];\n Int(6) m_u;\n Bool(3) b;\n Int test() {\n Int a;\n a = 3;\n m_i[0] = a;\n Int(2+3) exp;\n exp = 8;\n m_u = exp;\n m_i[1] = exp;\n return exp;\n}\n }\n");

      if(rtn1)
	return std::string("C.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3171_test_compiler_typebitsizeexpression)

} //end MFM
