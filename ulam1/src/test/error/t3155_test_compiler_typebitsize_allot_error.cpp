#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3155_test_compiler_typebitsize_allot_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./C.ulam:5:2: Warning: Bool Type with EVEN number of bits is internally inconsistent;  Reduced by one to 1 bits.
	./C.ulam:7:9: ERROR: Converting Int(32) to Int(3) requires explicit casting for operator=.
	./C.ulam:10:2: ERROR: Trying to exceed allotted bit size (71) for element C(UNKNOWN)<14> with 74 bits.
	./C.ulam:2:10: ERROR: CLASS (regular): C(UNKNOWN)<14> SIZED FAILED: 74.
      */
      return std::string("Ue_C { Bool(1) a(true);  Int(3) m_i[4](2,7,0,0);  Int(6) m_u(15);  Bool(3) b(true);  Int(32) test() {  m_i 0 [] 2 cast = m_u 15 cast = m_i 1 [] m_u cast = b m_i 0 [] cast = b m_i 0 [] cast &= a b cast = 0 return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("C.ulam","ulam 1;\n element C {\n Bool a;\n Int(3) m_i[24];\n Bool(2) b;\n Int test() {\n m_i[0] = 7;\n return 0;\n}\n }\n");

      if(rtn1)
	return std::string("C.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3155_test_compiler_typebitsize_allot_error)

} //end MFM
