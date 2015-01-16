#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3207_test_compiler_elementandquark_inside_a_quark)
  {
    std::string GetAnswerKey()
    {
      // output of generated code compiled with:
      // g++ -ansi -Wall -pedantic -c Ue_102283Foo_main.cpp
      // g++ -ansi -Wall -pedantic -o main Ue_102283Foo_main.cpp
      // ./main
      //
      //Int Arg: -1
      //Int(4) Arg: 0xf
      //Unsigned Arg: 3
      //Unsigned Arg: 0
      //Int(3) Arg: 0x3
      //Int Arg: 3

      //constant fold minus
      return std::string("Exit status: -1\nUe_Foo { Bool(1) b(false);  System m_s();  Gah m_gah( Bool(1) b(false);  System m_s();  Int(3) a(3);  Int(3) d(3); );  Int(3) i(0);  Int(3) j(7);  Bar m_bar( Bool(1) b(true);  System m_s();  Unsigned(3) x(3);  Unsigned(3) y(0);  Gah m_gah( Bool(1) b(false);  System m_s();  Int(3) a(0);  Int(3) d(0); ); );  Int(32) test() {  i 0 cast = j ( i cast 1 cast -b )update = m_s ( j cast )print . m_s ( j cast )print . m_bar ( 3 cast -1 cast )set . m_gah ( 4 cast 4 cast )set . j cast return } }\nUq_Bar { Bool(1) b(false);  System m_s();  Unsigned(3) x(0);  Unsigned(3) y(7);  Gah m_gah( Bool(1) b(false);  System m_s();  Int(3) a(3);  Int(3) d(3); );  <NOMAIN> }\nUq_System { <NOMAIN> }\nUq_Gah { Bool(1) b(false);  System m_s();  Int(3) a(0);  Int(3) d(7);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //note: unexpected, yet correct: 4 as Int(3) prints as -4 (Gah); -1 cast to Unsigned prints as 0 (Bar);
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nuse Gah;\nuse System;\nelement Foo {\nSystem m_s;\nBool b;\nInt(3) i, j;\nGah m_gah;\nBar m_bar;\nInt(3) update(Int x)\n{\nreturn x;\n}\nInt test(){\ni = 0;\nj = update(i - 1);\nm_s.print((Int) j);\nm_s.print((Int(4)) j);\nm_bar.set(3,-1);\nm_gah.set(4,4);\nreturn j;\n}\n}\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nuse System;\nuse Gah;\nquark Bar{\nSystem m_s;\nBool b;\nUnsigned(3) x, y;\nGah m_gah;\nInt toInt(){\nif(b)\nreturn (x * 4) / y;\nelse\nreturn 0;\n}\nVoid set(Int xarg, Int yarg){\nx=xarg;\ny=yarg;\nm_s.print((Unsigned) x);\nm_s.print((Unsigned) y);\n\nif(yarg)\n{\nb=true;\n}\nelse{\nb=false;\n}\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      bool rtn4 = fms->add("Gah.ulam","ulam 1;\nuse System;\nquark Gah{\nSystem m_s;\nBool b;\nInt(3) a, d;\nInt toInt(){\nif(a | 1)\nreturn (a + 4) * d;\nreturn a;\n}\nVoid set(Int xarg, Int yarg){\na=xarg;\nd=yarg;\nm_s.print(a);\nm_s.print((Int) d);\n}\n}\n");

      if(rtn1 && rtn2 && rtn3 && rtn4)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3207_test_compiler_elementandquark_inside_a_quark)

} //end MFM


