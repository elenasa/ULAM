#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3171_test_compiler_typebitsizeexpression)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_C { Bool(1) a(false);  Int(3) m_i[4](3,0,0,0);  Int(6) m_u(8);  Bool(3) b(false);  Int(32) test() {  Int(32) a;  a 3 = m_i 0 [] a cast = Int(5) exp;  exp 8 cast = m_u exp cast = m_i 1 [] exp cast = exp cast return } }\nExit status: 8");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      // doesn't work with variables, local or data members: Int(a), Int(2+a), OR Int(2+m_i[0]);
      bool rtn1 = fms->add("C.ulam","ulam 1; element C { Bool a; Int(3) m_i[4]; Int(6) m_u; Bool(3) b; Int test() { Int a; a = 3; m_i[0] = a; Int(2+3) exp; exp = 8; m_u = exp; m_i[1] = exp; return exp;} }");
      
      if(rtn1)
	return std::string("C.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3171_test_compiler_typebitsizeexpression)
  
} //end MFM


