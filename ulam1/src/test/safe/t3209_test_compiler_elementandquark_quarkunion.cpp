#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3209_test_compiler_elementandquark_quarkunion)
  {
    std::string GetAnswerKey()
    {

      /* gen code output:
	 Int(4) Arg: 0x1
	 Bool(3) Arg: 0x0 (false)
	 Int(4) Arg: 0xf
	 Bool(3) Arg: 0x7 (true)
      */

      return std::string("Ue_Foo { Bool(3) b(false);  Bool(1) e(false);  System s();  Int(4) i(1);  Gah m_gah( Bool(3) b(true);  Int(4) a(15);  Unary(7) c(4);  Unsigned(6) d(60);  Bits(2) e(3);  Bool(1) f(true); );  Int(32) test() {  i 1 cast = m_gah ( i cast )set . s ( m_gah a . )print . s ( m_gah b . )print . m_gah ( true cast )set . s ( m_gah a . )print . s ( m_gah b . )print . i cast return } }\nExit status: 1");
    }
    
    std::string PresetTest(FileManagerString * fms)
    {      
      //note: unexpected, yet correct: 4 as Int(3) prints as -4 (Gah); -1 cast to Unsigned prints as 0 (Bar);
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Gah;\nuse System;\nelement Foo {\nSystem s;\nBool(3) b;\nInt(4) i;\nGah m_gah;\nBool e;\nInt test(){\ni = 1;\nm_gah.set((Int) i);\ns.print(m_gah.a);\ns.print(m_gah.b);\nm_gah.set(true);\ns.print(m_gah.a);\ns.print(m_gah.b);\nreturn i;\n}\n}\n");

      bool rtn4 = fms->add("Gah.ulam","ulam 1;\nunion Gah{\nBool(3) b;\nInt(4) a;\nUnary(7) c;\nUnsigned(6) d;\nBits(2) e;\nBool f;\nVoid set(Int xarg){\na=xarg;\n}\nVoid set(Unsigned xarg){\nd=xarg;\n}\nVoid set(Unary xarg){\nc=xarg;\n}\nVoid set(Bool xarg){\nb=xarg;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");     

      if(rtn1 && rtn3 && rtn4)
	return std::string("Foo.ulam");
      
      return std::string("");
    }
  }
  
  ENDTESTCASECOMPILER(t3209_test_compiler_elementandquark_quarkunion)
  
} //end MFM


