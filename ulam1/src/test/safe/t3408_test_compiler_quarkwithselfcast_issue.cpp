#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3408_test_compiler_quarkwithselfcast_issue)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int(3) Arg: 0x1
	 Int(4) Arg: 0x2
	 Int(3) Arg: 0x2
	 Int(4) Arg: 0x3
	 Int(3) Arg: 0x2
	 Int(4) Arg: 0x3
      */
      //eval self doesn't reflect correctly (see gen output): is an atom for quark's function call hidden arg
      return std::string("Exit status: 0\nUe_Foo { IntXY(3u,4u) n( constant Unsigned(32) xbits = 3;  constant Unsigned(32) ybits = 4;  typedef IntXY(3u,4u) Self;  typedef Int(3) XCoord;  typedef Int(4) YCoord;  Int(3) x(2);  Int(4) y(3); );  Int(32) test() {  System s;  IntXY(3u,4u) m;  m n ( 1 2 )make . = s ( m x . )print . s ( m y . )print . m n ( 2 3 )fromXY . = s ( n x . )print . s ( n y . )print . s ( m x . )print . s ( m y . )print . 0 return } }\nUq_IntXY { constant Unsigned(32) xbits = NONREADYCONST;  constant Unsigned(32) ybits = NONREADYCONST;  typedef IntXY(xbits,ybits) Self;  typedef Int(UNKNOWN) XCoord;  typedef Int(UNKNOWN) YCoord;  Int(UNKNOWN) x(0);  Int(UNKNOWN) y(0);  <NOMAIN> }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("Foo.ulam", "ulam 1;\nuse IntXY;\nuse System;\n element Foo {\nIntXY(3u,4u) n;\n Int test() {\nSystem s;\n IntXY(3u,4u) m;\n m = n.make(1,2);\n s.print(m.x);\n s.print(m.y);\n m = n.fromXY(2,3);\n s.print(n.x);\n s.print(n.y);\n s.print(m.x);\n s.print(m.y);\n return 0;\n}\n }\n");

      bool rtn1 = fms->add("IntXY.ulam", "ulam 1;\nquark IntXY(Unsigned xbits, Unsigned ybits) {\n typedef IntXY(xbits, ybits) Self;\n typedef Int(xbits) XCoord;\n typedef Int(ybits) YCoord;\n XCoord x;\n YCoord y;\n Self fromXY(Int x, Int y) {\n init(x,y);\n return (Self) self;\n}\n Self make(Int x, Int y) {\n Self ret;\n ret.init(x,y);\n return ret;\n}\n  Self makeAs(Int x, Int y) {\n if(self as Self){\n self.init(x,y);\n return self;\n}\n return (Self) self;\n}\n Void init(Int ax, Int ay) {\n x = ax;\n y = ay;\n if(x == x.minof) ++x;\n if(y == y.minof) ++ y;\n}\n  }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3408_test_compiler_quarkwithselfcast_issue)

} //end MFM
