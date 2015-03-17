#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3200_test_compiler_elementandquark_twoquarkscasttoint_compilable)
  {
    std::string GetAnswerKey()
    {
      //developed by Dave (10092014) in c++ and then converted to Ulam.
      //Int(4) maxes out at 7, not 12.
      //added prints
      /* gen code output:
	 Unsigned Arg: 1
	 Unsigned Arg: 2
	 Unsigned Arg: 3
	 Unsigned Arg: 0
	 Int(4) Arg: 0x6
	 Int(4) Arg: 0x7
      */

      //constant fold minus
      return std::string("Exit status: 0\nUe_Foo { System s();  Bar bar1( Bool(1) b(false);  Unsigned(3) x(1);  Unsigned(3) y(2); );  Bar bar2( Bool(1) b(false);  Unsigned(3) x(3);  Unsigned(3) y(0); );  Int(4) i(6);  Int(4) j(7);  Int(32) test() {  Int(2) d;  d 1 cast = bar1 x . d cast = bar1 y . 2u cast = bar2 x . 3u cast = bar2 y . 0u cast = i bar1 ( )toInt . cast = j bar2 ( )toInt . cast = s ( bar1 x . cast )print . s ( bar1 y . cast )print . s ( bar2 x . cast )print . s ( bar2 y . cast )print . s ( i )print . s ( j )print . 0 return } }\nUq_System { <NOMAIN> }\nUq_Bar { Bool(1) b(false);  Unsigned(3) x(1);  Unsigned(3) y(2);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // fixed a bug that didn't address different int bit sizes automatically during casting
      // now that we believe toInt works, let's simplify Foo (remove the Int data member) for code gen testing
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nBar bar1;\nBar bar2;\nInt test(){\nbar1.x = 1;\nbar1.y = 2;\nbar2.x = 3;\nbar2.y = 0;\nreturn 0;}\n}\n");

      // use a variable for rhs of bar1; added system prints
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Bar;\nelement Foo {\nSystem s;\nBar bar1;\nBar bar2;\nInt(4) i, j;\nInt test(){\nInt(2) d;\nd = 1;\nbar1.x = d;\nbar1.y = 2u;\nbar2.x = 3u;\nbar2.y = 0u;\ni = bar1;\nj = bar2;\ns.print((Unsigned) bar1.x);\ns.print((Unsigned) bar1.y);\ns.print((Unsigned) bar2.x);\ns.print((Unsigned) bar2.y);\n\ns.print(i);\ns.print(j);\nreturn 0;}\n}\n");

      //note: don't have <<2, so substituted *4; use 3 bits so not to cross word boundary
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nBool b;\nUnsigned(3) x, y;\nInt toInt(){\nreturn (x * 4) + y;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3200_test_compiler_elementandquark_twoquarkscasttoint_compilable)

} //end MFM
