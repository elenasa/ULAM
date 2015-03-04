#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3333_test_compiler_elementwithclassparameters_deferredfuncargs)
  {
    //informed by t3332
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 7
	 Int Arg: 3
      */
      return std::string("Exit status: 7\nUe_T { Int(32) i(7);  Int(32) j(3);  System s();  Int(32) test() {  Int(3) a;  Int(4) b;  S(3,1) m;  i m ( b )func . = s ( i )print . j m ( a )func . = s ( j )print . i return } }\nUq_S { constant Int(CONSTANT) x = NONREADYCONST;  constant Int(CONSTANT) y = NONREADYCONST;  Int(UNKNOWN) i(0);  Int(UNKNOWN) j(0);  <NOMAIN> }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      /* S(3,2) m; gives errors:
	 T.ulam:13:7: ERROR: (1) <func> has no defined function with 1 matching argument types: Int(4), and cannot be called, while compiling class: T.
      */

      // S(3,1) m; works!!
      bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse S;\nuse System;\n element T{\nSystem s;\nInt i,j;\nInt test(){\nInt(3) a;\nInt(4) b;\nS(3,1) m;\ni = m.func(b);\ns.print(i);\n j = m.func(a);\ns.print(j);\n return i;\n}\n }\n");

      bool rtn1 = fms->add("S.ulam"," ulam 1;\nquark S(Int x, Int y){\nInt(x+y) i,j;\n Int func(Int(x+y) arg){\nreturn arg.maxof;\n}\nInt func(Int(x) arg){\nreturn arg.maxof;\n}\nInt func(Int(y) arg){\nreturn arg.maxof;\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
      	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3333_test_compiler_elementwithclassparameters_deferredfuncargs)

} //end MFM
