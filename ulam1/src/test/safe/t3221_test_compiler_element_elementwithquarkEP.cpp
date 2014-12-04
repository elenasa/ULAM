#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3221_test_compiler_element_elementwithquarkEP)
  {
    std::string GetAnswerKey()
    {
      return std::string("Ue_Foo { Bool(3) b(false);  System s();  Bool(1) sp(false);  Bool(3) c(false);  Bar bar( Bool(1) val_b[3](false,true,false); );  Bool(1) last(false);  Int(32) test() {  Int(32) i;  i 1 cast = bar poochance ( i )check . = poochance m_bar . bar = s ( poochance m_bar val_b i [] . . )assert . poochance m_bar val_b i [] . . cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Poo;\nuse Bar;\nelement Foo {\nSystem s;\nBool(1) sp;\nBool(3) b,c;\n element Poo poochance;\nBar bar;\nBool last;\nInt test() { \nInt i;\n i =1;\nbar = poochance.check(i);\npoochance.m_bar = bar;\n s.assert(poochance.m_bar.val_b[i]);\nreturn (Int) poochance.m_bar.val_b[i];\n }\n }\n");
    
      bool rtn4 = fms->add("Poo.ulam","ulam 1;\nuse Bar;\nuse System;\nelement Poo {\nSystem s;\nBool(3) sp;\nUnary(4) m_i;\nUnary(4) m_j;\nBits(4) m_bits;\nInt(4) m_k;\nBar m_bar;\nBar check(Int v) {\nBar b;\nb.val_b[v] = true;\nreturn b;}\n\n}\n");

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


