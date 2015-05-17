#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3155_test_compiler_typebitsize_allot_error)
  {
    std::string GetAnswerKey()
    {
      /*
	./C.ulam:1:45: Warning: Bool Type with EVEN number of bits is internally inconsistent; Reduced by one to 1 bits.
	:0:0: ERROR: Trying to exceed allotted bit size (71) for element C(UNKNOWN)<11> with 74 bits.
	./C.ulam:1:29: ERROR: Data member <m_i> of type: Int(3)[24] (UTI13) total size: 72 MUST fit into 32 bits; Local variables do not have this restriction.
      */
      return std::string("Ue_C { Bool(1) a(true);  Int(3) m_i[4](2,7,0,0);  Int(6) m_u(15);  Bool(3) b(true);  Int(32) test() {  m_i 0 [] 2 cast = m_u 15 cast = m_i 1 [] m_u cast = b m_i 0 [] cast = b m_i 0 [] cast &= a b cast = 0 return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("C.ulam","ulam 1; element C { Bool a; Int(3) m_i[24]; Bool(2) b; Int test() { m_i[0] = 7; return 0;} }");

      if(rtn1)
	return std::string("C.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3155_test_compiler_typebitsize_allot_error)

} //end MFM
