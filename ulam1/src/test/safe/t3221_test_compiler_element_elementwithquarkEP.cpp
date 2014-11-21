#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3221_test_compiler_element_elementwithquarkEP)
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
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nUnary(4) m_i;\nUnary(4) m_j;\nBits(4) m_bits;\nInt(4) m_k;\nBar m_bar;\nBar check(Int v) {\nBar b;\nb.val_b[1] = true;\nreturn b; }\nInt test() {\nm_bar = check(1);\nm_i = (Unary)((Int) m_bar);\nm_j = (Unary(4)) 7;\nm_bits = m_i;\nm_k = m_bits;\nreturn m_i;}\n}\n");
      //      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Poo;\nuse Bar;\nelement Foo {\nSystem s;\nBool(1) sp;\nBool(3) b,c;\n element Poo poochance;\nBar bar;\nBool last;\nInt test() { \nInt i;\n i =1;\nbar = poochance.check(i);\nBool ba;\n ba = poochance.m_bar.val_b[i];\n s.assert(ba);\nreturn (Int) poochance.m_bar.val_b[i];\n }\n }\n");

      //works!
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Poo;\nuse Bar;\nelement Foo {\nSystem s;\nBool(1) sp;\nBool(3) b,c;\n element Poo poochance;\nBar bar;\nBool last;\nInt test() { \nInt i;\n i =1;\nbar = poochance.check(i);\n s.assert(bar.val_b[i]);\nreturn (Int) poochance.m_bar.val_b[i];\n }\n }\n");

bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Poo;\nuse Bar;\nelement Foo {\nSystem s;\nBool(1) sp;\nBool(3) b,c;\n element Poo poochance;\nBar bar;\nBool last;\nInt test() { \nInt i;\n i =1;\nbar = poochance.check(i);\npoochance.m_bar = bar;\n s.assert(poochance.m_bar.val_b[i]);\nreturn (Int) poochance.m_bar.val_b[i];\n }\n }\n");

    
      bool rtn4 = fms->add("Poo.ulam","ulam 1;\nuse Bar;\nuse System;\nelement Poo {\nSystem s;\nBool(3) sp;\nUnary(4) m_i;\nUnary(4) m_j;\nBits(4) m_bits;\nInt(4) m_k;\nBar m_bar;\nBar check(Int v) {\nBar b;\nb.val_b[v] = true;\nreturn b;}\n\n}\n");

      //simplified test() for debug..
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nUnary(4) m_i;\nUnary(4) m_j;\nBits(4) m_bits;\nInt(4) m_k;\nBar m_bar;\nBar check(Int v) {\nBar b;\nb.val_b[1] = true;\nreturn b; }\nInt test() {\nm_i = (Unary)((Int) m_bar);\nreturn m_i;}\n}\n");
      
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nBool val_b[3];\nVoid reset(Bool b) {\nb = 0; }\nInt toInt(){\nreturn 3;}\n}\n");
      
      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3221_test_compiler_element_elementwithquarkEP)
  
} //end MFM


