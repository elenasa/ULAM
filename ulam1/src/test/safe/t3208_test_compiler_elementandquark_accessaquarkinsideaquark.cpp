#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3208_test_compiler_elementandquark_accessaquarkinsideaquark)
  {
    std::string GetAnswerKey()
    {
      //changed *'s to << and Int(3) to Int(4) to get rid of warningsl last output changed from 4 to 8.
      /* generated code output:
	 Int(3) Arg: 0x2
	 Int Arg: -2
	 Int(4) Arg: 0x8
      */

      //was:
      //Exit status: 0\nUe_Foo { Bool(1) b(false);  System m_s();  Int(3) i(0);  Int(3) j(4);  Bar m_bar( Bool(1) b(false);  System m_s();  Unsigned(3) x(0);  Unsigned(3) y(0);  Gah m_gah( Bool(1) b(false);  System m_s();  Int(3) a(2);  Int(3) d(6); ); );  Int(32) test() {  m_bar m_gah ( 2 -2 )set . . j m_bar m_gah . ( )toInt . cast = m_s ( j )print . 0 return } }\nUq_Bar { Bool(1) b(false);  System m_s();  Unsigned(3) x(0);  Unsigned(3) y(4);  Gah m_gah( Bool(1) b(false);  System m_s();  Int(3) a(0);  Int(3) d(0); );  <NOMAIN> }\nUq_System { <NOMAIN> }\nUq_Gah { Bool(1) b(false);  System m_s();  Int(3) a(0);  Int(3) d(4);  <NOMAIN> }

      //constant fold minus
      return std::string("Exit status: 0\nUe_Foo { Bool(1) b(false);  System m_s();  Int(4) i(0);  Int(4) j(8);  Bar m_bar( Bool(1) b(false);  System m_s();  Unsigned(4) x(0);  Unsigned(4) y(0);  Gah m_gah( Bool(1) b(false);  System m_s();  Int(4) a(2);  Int(4) d(14); ); );  Int(32) test() {  m_bar m_gah ( 2 -2 )set . . j m_bar m_gah . ( )toInt . cast = m_s ( j )print . 0 return } }\nUq_Bar { Bool(1) b(false);  System m_s();  Unsigned(4) x(0);  Unsigned(4) y(8);  Gah m_gah( Bool(1) b(false);  System m_s();  Int(4) a(0);  Int(4) d(0); );  <NOMAIN> }\nUq_System { <NOMAIN> }\nUq_Gah { Bool(1) b(false);  System m_s();  Int(4) a(0);  Int(4) d(8);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //note: unexpected, yet correct: 4 as Int(3) prints as -4 (Gah); -1 cast to Unsigned prints as 0 (Bar);
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nuse Gah;\nuse System;\nelement Foo {\nSystem m_s;\nBool b;\nInt(4) i, j;\nBar m_bar;\nInt test(){\nm_bar.m_gah.set(2,-2);\nj = m_bar.m_gah;\n m_s.print(j);\nreturn 0;\n}\n}\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nuse System;\nuse Gah;\nquark Bar{\nSystem m_s;\nBool b;\nUnsigned(4) x, y;\nGah m_gah;\nInt toInt(){\nif(b)\nreturn (x << 2) / y;\nelse\nreturn 0;\n}\nVoid set(Int xarg, Int yarg){\nx=xarg;\ny=yarg;\nm_s.print((Unsigned) x);\nm_s.print((Unsigned) y);\n\nif(yarg)\n{\nb=true;\n}\nelse{\nb=false;\n}\n}\n}\n");

      bool rtn4 = fms->add("Gah.ulam","ulam 1;\nuse System;\nquark Gah{\nSystem m_s;\nBool b;\nInt(4) a, d;\nInt toInt(){\nif(a | 1)\nreturn (a + 4) << d;\nreturn a;\n}\nVoid set(Int xarg, Int yarg){\na=xarg;\nd=yarg;\nm_s.print(a);\nm_s.print((Int) d);\n}\n}\n");


      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3208_test_compiler_elementandquark_accessaquarkinsideaquark)

} //end MFM
