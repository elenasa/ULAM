#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3409_test_compiler_quarkwithelementcast)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 1
	 Int(3) Arg: 0x1

      */
      //eval self doesn't reflect correctly (see gen output): is an atom for quark's function call hidden arg
      return std::string("Exit status: 0\nUe_Foo { System s();  IntXY(3u,4u) n( constant Unsigned(32) xbits = 3;  constant Unsigned(32) ybits = 4;  typedef IntXY(3u,4u) Self;  typedef Int(3) XCoord;  typedef Int(4) YCoord;  Int(3) x(1);  Int(4) y(2); );  Int(32) test() {  n ( 1 2 )init . Int(32) i;  i ( self cast )func = s ( i )print . s ( n x . )print . i return } }\nUq_IntXY { constant Unsigned(32) xbits = NONREADYCONST;  constant Unsigned(32) ybits = NONREADYCONST;  typedef IntXY(xbits,ybits) Self;  typedef Int(UNKNOWN) XCoord;  typedef Int(UNKNOWN) YCoord;  Int(UNKNOWN) x(0);  Int(UNKNOWN) y(0);  <NOMAIN> }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Foo.ulam", "ulam 1;\nuse IntXY;\nuse System;\n element Foo {\nSystem s;\n IntXY(3u,4u) n;\n Int func(IntXY(3,4) arg) {\n return arg.x;\n}\n Int test() {\n n.init(1,2);\n Int i = func((IntXY(3,4)) self);\n s.print(i);\n s.print(n.x);\n return i;\n}\n }\n");

      bool rtn1 = fms->add("IntXY.ulam", "ulam 1;\nquark IntXY(Unsigned xbits, Unsigned ybits) {\n typedef IntXY(xbits, ybits) Self;\n typedef Int(xbits) XCoord;\n typedef Int(ybits) YCoord;\n XCoord x;\n YCoord y;\n  Void init(Int ax, Int ay) {\n x = (XCoord) ax;\n y = (YCoord) ay;\n if(x == x.minof) ++x;\n if(y == y.minof) ++y;\n}\n  }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3409_test_compiler_quarkwithelementcast)

} //end MFM
