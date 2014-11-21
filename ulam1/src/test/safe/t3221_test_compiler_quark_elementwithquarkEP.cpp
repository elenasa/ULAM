#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3221_test_compiler_quark_elementwithquarkEP)
  {
    std::string GetAnswerKey()
    {
      //notes: double cast needed to cast quarks into anything other
      //than Int (see t3198 for the error test); m_j is saturated
      //(i.e. 7 bits into bitsize 4 is 4); and it's Bits that sees the
      //unary value of 3 as 7;

      //Ue_Foo { Unary(4) m_i(3);  Unary(4) m_j(4);  Bits(4) m_bits(7);  Int(4) m_k(7);  Bar m_bar( Bool(1) val_b[3](false,true,false); );  Int(32) test() {  m_bar ( 1 cast )check = m_i m_bar ( )toInt . cast cast cast = m_j 7 cast = m_bits m_i cast = m_k m_bits cast = m_i cast return } }\nExit status: 3
      //extra casts around return stmt..
      // code gen output:
      //Unsigned Arg: 3
      //Unsigned Arg: 4
      //Unsigned Arg: 7
      //Unsigned Arg: 7

      return std::string("Ue_Foo { System s();  Bool(3) sp(false);  Unary(4) m_i(3);  Unary(4) m_j(4);  Bits(4) m_bits(7);  Int(4) m_k(7);  Bar m_bar( Bool(1) val_b[3](false,true,false); );  Int(32) test() {  m_bar ( 1 cast )check = m_i m_bar ( )toInt . cast cast cast cast = m_j 7 cast = m_bits m_i cast = m_k m_bits cast = s ( m_i cast )print . s ( m_j cast )print . s ( m_bits cast )print . s ( m_k cast )print . m_i cast return } }\nExit status: 3");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nBool val_b[3];\nVoid reset(Bool b) {\nb = 0; }\nInt toInt(){\nreturn 3;}\n}\n");
      
      if(rtn2)
	return std::string("Bar.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3221_test_compiler_quark_elementwithquarkEP)
  
} //end MFM


