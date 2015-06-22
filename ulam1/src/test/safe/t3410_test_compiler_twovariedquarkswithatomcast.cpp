#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3410_test_compiler_twovariedquarkswithatomcast)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Unsigned Arg: 1
	 Unsigned Arg: 2
	 Unsigned Arg: 2
	 Unsigned Arg: 3
	 Unsigned Arg: 2
	 Unsigned Arg: 3
      */
      //eval self doesn't reflect correctly (see gen output): is an atom for quark's function call hidden arg
      return std::string("Exit status: 0\nUe_Foo { IntXY(1u,4u) q( constant Unsigned(32) xbits = 1;  constant Unsigned(32) ybits = 4;  typedef IntXY(1u,4u) Self;  typedef Unsigned(1) XCoord;  typedef Unsigned(4) YCoord;  Unsigned(1) x(0);  Unsigned(4) y(0); );  IntXY(3u,4u) n( constant Unsigned(32) xbits = 3;  constant Unsigned(32) ybits = 4;  typedef IntXY(3u,4u) Self;  typedef Unsigned(3) XCoord;  typedef Unsigned(4) YCoord;  Unsigned(3) x(2);  Unsigned(4) y(3); );  IntXY(2u,3u) p( constant Unsigned(32) xbits = 2;  constant Unsigned(32) ybits = 3;  typedef IntXY(2u,3u) Self;  typedef Unsigned(2) XCoord;  typedef Unsigned(3) YCoord;  Unsigned(2) x(0);  Unsigned(3) y(0); );  Int(32) test() {  System s;  IntXY(3u,4u) m;  m n ( 1 2 )make . = s ( m x . cast )print . s ( m y . cast )print . m n ( 2 3 )fromXY . = s ( n x . cast )print . s ( n y . cast )print . s ( m x . cast )print . s ( m y . cast )print . 0 return } }\nUq_IntXY { constant Unsigned(32) xbits = NONREADYCONST;  constant Unsigned(32) ybits = NONREADYCONST;  typedef IntXY(xbits,ybits) Self;  typedef Unsigned(UNKNOWN) XCoord;  typedef Unsigned(UNKNOWN) YCoord;  Unsigned(UNKNOWN) x(0);  Unsigned(UNKNOWN) y(0);  <NOMAIN> }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by t3408: here, n is surrounded by two other IntXY data members q and p.
      bool rtn2 = fms->add("Foo.ulam", "ulam 1;\nuse IntXY;\nuse System;\n element Foo {\nIntXY(1,4) q;\nIntXY(3u,4u) n;\nIntXY(2,3) p;\n Int test() {\nSystem s;\n IntXY(3u,4u) m;\n m = n.make(1,2);\n s.print((Unsigned) m.x);\n s.print((Unsigned) m.y);\n m = n.fromXY(2,3);\n s.print((Unsigned) n.x);\n s.print((Unsigned) n.y);\n s.print((Unsigned) m.x);\n s.print((Unsigned) m.y);\n return 0;\n}\n }\n");

      //changed XCoord and YCoord to Unsigned types, because of the error below. Better since class args are Unsigned; used Int to print gen output with various bitsize prints defined. Added maxof test to do --x;
      //./IntXY.ulam:27:19: ERROR: Attempting to fit a constant <1> into a smaller bit size type, LHS: Int(1), for binary operator+= .
      bool rtn1 = fms->add("IntXY.ulam", "ulam 1;\nquark IntXY(Unsigned xbits, Unsigned ybits) {\n typedef IntXY(xbits, ybits) Self;\n typedef Unsigned(xbits) XCoord;\n typedef Unsigned(ybits) YCoord;\n XCoord x;\n YCoord y;\n Self fromXY(Int x, Int y) {\n init(x,y);\n return (Self) self;\n}\n Self make(Int x, Int y) {\n Self ret;\n ret.init(x,y);\n return ret;\n}\n  Self makeAs(Int x, Int y) {\n if(self as Self){\n self.init(x,y);\n return self;\n}\n return (Self) self;\n}\n Void init(Int ax, Int ay) {\n x = (XCoord) ax;\n y = (YCoord) ay;\n if(x == x.minof) ++x;\n if(y == y.minof) ++y;\n if(x == x.maxof) --x;\n if(y == y.maxof) --y;\n}\n  }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }


  ENDTESTCASECOMPILER(t3410_test_compiler_twovariedquarkswithatomcast)

} //end MFM
