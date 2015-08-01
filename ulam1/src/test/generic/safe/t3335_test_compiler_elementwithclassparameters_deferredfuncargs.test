#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3335_test_compiler_elementwithclassparameters_deferredfuncargs)
  {
    //informed by t3333
    std::string GetAnswerKey()
    {
      //with implicit b casted to func; it works!!
      return std::string("Exit status: 15\nUe_T { Int(32) i(15);  Int(32) j(3);  System s();  Int(32) test() {  Int(3) a;  Int(4) b;  S(3,2) m;  i m ( b cast )func . = s ( i )print . j m ( a )func . = s ( j )print . i return } }\nUq_S { constant Int(32) x = NONREADYCONST;  constant Int(32) y = NONREADYCONST;  Int(UNKNOWN) i(0);  Int(UNKNOWN) j(0);  <NOMAIN> }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      /* S(3,2) m; gaves errors:
	 ./S.ulam:5:1: ERROR: Function 'func''s Return type's: Int(32) does not match incomplete resulting type.
      */

      // note: S(3,1) m; works!!
      bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse S;\nuse System;\n element T{\nSystem s;\nInt i,j;\nInt test(){\nInt(3) a;\nInt(4) b;\nS(3,2) m;\ni = m.func(b);\ns.print(i);\n j = m.func(a);\ns.print(j);\n return i;\n}\n }\n");

      bool rtn1 = fms->add("S.ulam"," ulam 1;\nquark S(Int x, Int y){\nInt(x+y) i,j;\n Int func(Int(x+y) arg){\nreturn arg.maxof;\n}\nInt func(Int(x) arg){\nreturn arg.maxof;\n}\nInt func(Int(y) arg){\nreturn arg.maxof;\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
      	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3335_test_compiler_elementwithclassparameters_deferredfuncargs)

} //end MFM
