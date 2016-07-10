#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3154_test_compiler_typebitsize_evenbool_warn)
  {
    std::string GetAnswerKey()
    {
      //./C.ulam:5:2: Warning: Bool Type with EVEN number of bits is internally inconsistent; Reduced by one to 3 bits.
      return std::string("Exit status: 0\nUe_C { Bool(1) a(true);  Int(3) m_i[4](2,3,0,0);  Int(6) m_u(15);  Bool(3) b(true);  Int(32) test() {  m_i 0 [] 2 cast = m_u 15 cast = m_i 1 [] m_u cast = b m_i 0 [] cast = b m_i 0 [] cast &= a b cast = 0 return } }");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //      bool rtn1 = fms->add("C.ulam","ulam 1; element C { Bool a; Int(3) m_i[23]; Bool b; Int test() { m_i[0] = 7; return 0;} }");
      //bool rtn1 = fms->add("C.ulam","ulam 1; element C { Bool a; Int(3) m_i[10]; Int(6) m_u; Bool(3) b; Int test() { m_i[0] = 2; b = m_i[0]; a = b; return 0;} }");

      // note: 15 fits into 3 bits and turns into 7;
      bool rtn1 = fms->add("C.ulam","ulam 1; element C {\n Bool a;\n Int(3) m_i[4];\n Int(6) m_u;\n Bool(4) b;\n Int test() {\n m_i[0] = 2;\n m_u = 15;\n m_i[1] = m_u;\n b = m_i[0];\n b &= m_i[0];\n a = b;\n return 0;\n}\n }\n");

      if(rtn1)
	return std::string("C.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3154_test_compiler_typebitsize_evenbool_warn)

} //end MFM
