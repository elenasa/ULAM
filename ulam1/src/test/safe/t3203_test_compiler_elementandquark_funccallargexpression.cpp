#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3203_test_compiler_elementandquark_funccallargexpression)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int(4) Arg: 0x2
	 Int(4) Arg: 0x0
	 Int(4) Arg: 0x7
      */

      //developed by Dave (10092014) in c++ and then converted to Ulam.

      //constant fold minus
      return std::string("Exit status: 0\nUe_Foo { System s();  Bar bar1( Bool(1) b(true);  Unsigned(3) x(1);  Unsigned(3) y(2); );  Bar bar2( Bool(1) b(false);  Unsigned(3) x(3);  Unsigned(3) y(0); );  Int(4) i(2);  Int(4) j(7);  Int(32) test() {  Int(32) d;  d 1 = bar1 ( d 2 )set . bar2 ( 3 0 )set . i bar1 ( )toInt . cast cast = s ( i )print . j bar2 ( )toInt . cast cast = s ( j )print . j ( i 7 cast +b )update = s ( j )print . 0 return } }\nUq_System { <NOMAIN> }\nUq_Bar { Bool(1) b(true);  Unsigned(3) x(1);  Unsigned(3) y(2);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {

      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Bar;\nelement Foo {\nSystem s;\nBar bar1;\nBar bar2;\nInt(4) i, j;\nInt(4) update(Int(4) x)\n{\nreturn x;\n}\nInt test(){\nInt d;\nd = 1;\nbar1.set(d,2);\nbar2.set(3,0);\ni = (Int(4)) bar1;\ns.print(i);\nj = (Int(4)) bar2;\ns.print(j);\nj = update(i + 7);\ns.print(j);\nreturn 0;}\n}\n");

      //note: don't have <<2, so substituted *4; got it now! changes the answer too..
      // use set function; test bool to avoid divide by zero
      //note: don't have <<2, so substituted *4; use 3 bits so not to cross word boundary

      //./Bar.ulam:14:1: ERROR: Converting Int(32) to Bool(1) requires a logical comparison for if.
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nBool b;\nUnsigned(3) x, y;\nInt toInt(){\nif(b)\nreturn (Int) (x << 2) / y;\nelse\nreturn 0;\n}\nVoid set(Int xarg, Int yarg){\nx=(Unsigned(3)) xarg;\ny=(Unsigned(3)) yarg;\nif(yarg!=0){\nb=true;\n}\nelse{\nb=false;\n}\n}\n}\n");


      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");


      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3203_test_compiler_elementandquark_funccallargexpression)

} //end MFM
