#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3202_test_compiler_elementandquark_ifelsefunccall_compilable)
  {
    std::string GetAnswerKey()
    {
      //developed by Dave (10092014) in c++ and then converted to Ulam.
      // with printf in NodePrograms enabled for this test:
      // Bar1 toInt = 2
      // Bar2 toInt = 0

      /* gen code output:
	 Int(4) Arg: 0x2
	 Int(4) Arg: 0x0
      */

      // constant fold minus
      return std::string("Exit status: 0\nUe_Foo { System s();  Bar bar1( Bool(1) b(true);  Unsigned(3) x(1);  Unsigned(3) y(2); );  Bar bar2( Bool(1) b(false);  Unsigned(3) x(3);  Unsigned(3) y(0); );  Int(4) i(2);  Int(4) j(0);  Int(32) test() {  Int(32) d;  d 1 = bar1 ( d 2 )set . bar2 ( 3 0 )set . i bar1 ( )toInt . cast cast = j bar2 ( )toInt . cast cast = s ( i )print . s ( j )print . 0 return } }\nUq_System { <NOMAIN> }\nUq_Bar { Bool(1) b(true);  Unsigned(3) x(1);  Unsigned(3) y(2);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //note: don't have printf's yet so comments for now
      // printf(\"%d\n\", (Int) bar1);\nprintf(\"%d\n\", (Int) bar2);\n STARS AND SLASHES OH MY!!!

      // fixed a bug that didn't address different int bit sizes automatically during casting

      // now that we believe toInt works, let's simplify Foo (remove the Int data member) for code gen testing
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nBar bar1;\nBar bar2;\nInt test(){\nbar1.x = 1;\nbar1.y = 2;\nbar2.x = 3;\nbar2.y = 0;\nreturn 0;}\n}\n");

      // use a variable for rhs of bar1
      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nBar bar1;\nBar bar2;\nInt(4) i, j;\nInt test(){\nInt(2) d;\nd = 1;\nbar1.x = d;\nbar1.y = 2;\nbar2.x = 3;\nbar2.y = 0;\ni = bar1;\nj = bar2;\n\nreturn 0;}\n}\n");

      //bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse Bar;\nelement Foo {\nBar bar1;\nBar bar2;\nInt(4) i, j;\nInt test(){\nInt d;\nd = 1;\nbar1.set(d,2);\nbar2.set(3,0);\ni = bar1;\nj = bar2;\n\nreturn 0;}\n}\n");
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Bar;\nelement Foo {\nSystem s;\nBar bar1;\nBar bar2;\nInt(4) i, j;\nInt test(){\nInt d;\nd = 1;\nbar1.set(d,2);\nbar2.set(3,0);\ni = (Int(4)) bar1;\nj = (Int(4)) bar2;\ns.print(i);\ns.print(j);\nreturn 0;}\n}\n");

      //note: don't have <<2, so substituted *4; got it now! changes the answer too..
      // use set function; test bool to avoid divide by zero
      //note: don't have <<2, so substituted *4; use 3 bits so not to cross word boundary
      //bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nBool b;\nUnsigned(3) x, y;\nInt toInt(){\nif(b)\nreturn (x * 4) / y;\nelse\nreturn 0;\n}\nVoid set(Int xarg, Int yarg){\nx=xarg;\ny=yarg;\nif(yarg){\nb=true;\n}\nelse{\nb=false;\n}\n}\n}\n");
      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\nquark Bar {\nBool b;\nUnsigned(3) x, y;\nInt toInt(){\nif(b)\nreturn (Int) ((Bits(3)) x << 2) / y;\nelse\nreturn 0;\n}\nVoid set(Int xarg, Int yarg){\nx=(Unsigned(3)) xarg;\ny=(Unsigned(3)) yarg;\nif(yarg!=0){\nb=true;\n}\nelse{\nb=false;\n}\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3202_test_compiler_elementandquark_ifelsefunccall_compilable)

} //end MFM
