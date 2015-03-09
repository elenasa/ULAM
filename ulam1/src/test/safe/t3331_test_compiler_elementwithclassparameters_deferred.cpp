#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3331_test_compiler_elementwithclassparameters_deferred)
  {
    //informed by t3330
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Unsigned Arg: 6
	 Unsigned Arg: 3
	 Bool(3) Arg: 0x0 (false)
	 Unsigned Arg: 70
	 Unsigned Arg: 70
	 Bool(3) Arg: 0x7 (true)
      */
      //Constants have explicit types
      return std::string("Exit status: 6\nUe_T { Bool(3) b(false);  Unsigned(32) i(6);  Unsigned(32) j(70);  System s();  Bool(3) c(true);  Int(32) test() {  Unsigned(32) x;  Unsigned(32) y;  S(1,2) m;  i 6u cast = x m ( )func . cast = b i x == cast = s ( i )print . s ( x )print . s ( b )print . j 70u cast = y 70u cast = c j y == cast = s ( j )print . s ( y )print . s ( c )print . i cast return } }\nUq_System { <NOMAIN> }\nUq_S { constant Int(32) x = NONREADYCONST;  constant Int(32) y = NONREADYCONST;  Int(UNKNOWN) i(0);  Int(UNKNOWN) j(0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //rtn2 and rtn1 here match the answer perfectly!
      //bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse System;\nuse S;\n element T{\nSystem s;\nUnsigned i,j;\nBool(3) b,c;\n Int test(){\nUnsigned x,y;\n S(1,2) m;\ni = m.sizeof;\n x = m.func(); b = (i == x);\n s.print(i);\ns.print(x);\n s.print(b);\n j = self.sizeof;\ny = T.sizeof;\n c = (j == y);\n s.print(j);\n s.print(y);\n s.print(c);\n return i;\n}\n }\n");
      //bool rtn1 = fms->add("S.ulam"," ulam 1;\nquark S(Int x, Int y){\nInt(x+y) i,j;\n Int func(){\nreturn (x + y);\n}\n }\n");

      // here we set i using S(1,2).sizeof
      bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse System;\nuse S;\n element T{\nSystem s;\nUnsigned i,j;\nBool(3) b,c;\n Int test(){\nUnsigned x,y;\n S(1,2) m;\ni = S(1,2).sizeof;\n x = m.func(); b = (i == x);\n s.print(i);\ns.print(x);\n s.print(b);\n j = self.sizeof;\ny = T.sizeof;\n c = (j == y);\n s.print(j);\n s.print(y);\n s.print(c);\nreturn i;\n}\n }\n");

      //infinite loop 'S(x+y,n) s;' with x+y as class arg!
      //note: quark self.sizeof returns 96 (an atom's size).
      //note: S(x,y).sizeof in S returns UNKNOWN (-2 == 2147483647).
      //i.sizeof "Invalid Statement (possible missing semicolon)"; though in T, b.sizeof returns 3;
      //Int(x+y).sizeof "Invalid Statement (possible missing semicolon)"; though in T, Bool(3) returns 3;
      bool rtn1 = fms->add("S.ulam"," ulam 1;\nquark S(Int x, Int y){\nInt(x+y) i,j;\n Int func(){\nreturn  /* S(x,y).sizeof*/ (x + y) /* Int(x+y).sizeof */ /* i.sizeof*/;\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3331_test_compiler_elementwithclassparameters_deferred)

} //end MFM
