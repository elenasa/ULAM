#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3334_test_compiler_elementwithclassparameters_deferredfuncargs_duperror)
  {
    //informed by t3333
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 7
      */
      return std::string("Exit status: 7\nUe_T { Int(32) j(7);  System s();  Int(32) test() {  Int(4) b;  S(2,2) n;  j n ( b )func . = s ( j )print . j return } }\nUq_S { constant Int(CONSTANT) x = NONREADYCONST;  constant Int(CONSTANT) y = NONREADYCONST;  Int(UNKNOWN) i(0);  Int(UNKNOWN) j(0);  <NOMAIN> }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // ERROR: Check overloading function: <func> has a duplicate definition: Uf_4funcUt_10123Int, while compiling class: S(2,2).

      bool rtn2 = fms->add("T.ulam"," ulam 1;\nuse S;\nuse System;\n element T{\nSystem s;\nInt j;\nInt test(){\nInt(4) b;\nS(2,2) n;\nj = n.func(b);\n s.print(j);\n return j;\n}\n }\n");

      bool rtn1 = fms->add("S.ulam"," ulam 1;\nquark S(Int x, Int y){\nInt(x+y) i,j;\n Int func(Int(x+y) arg){\nreturn arg.maxof;\n}\nInt func(Int(x) arg){\nreturn arg.maxof;\n}\nInt func(Int(y) arg){\nreturn arg.maxof;\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
      	return std::string("T.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3334_test_compiler_elementwithclassparameters_deferredfuncargs_duperror)

} //end MFM
