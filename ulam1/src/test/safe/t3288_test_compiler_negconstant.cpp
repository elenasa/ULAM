#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3288_test_compiler_negconstant)
  {
    std::string GetAnswerKey()
    {
      /* gen output:
	 Int(3) Arg: 0x4  (not sign extended!)
	 Int Arg: -4
	 Int Arg: 3
	 assert: arg is 1
	 after assert's abort: arg is 1
	 Int(3) Arg: 0x3
	 Unsigned Arg: 3 (**note: use to be 4)
      */
      return std::string("Exit status: 3\nUe_Fu { System s();  Int(3) n(4);  Int(3) m(3);  Unsigned(3) u(3);  Int(32) test() {  n -4 cast = s ( n )print . s ( -4 cast )print . s ( 3 cast )print . s ( n -4 == )assert . m n - = s ( m )print . u n - cast = s ( u cast )print . m cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nuse System;\nelement Fu {\nSystem s;\nInt(3) n,m;\nUnsigned(3) u;\n Int test(){\n n = -4;\n s.print(n);\n s.print( (Int) n.minof);\n s.print( (Int) n.maxof);\n s.assert(n == n.minof);\n m = -n;\n s.print(m);\n u = (Unsigned(3)) -n; s.print((Unsigned) u);\nreturn m;\n}\n}\n");

      // flip flops problems with t3181; n.minof should be Int, not Int(3) casting constant to first "match". search matchingFunctionName strictly first, then flexibly. Tue Mar 17 14:18:49 2015
      //bool rtn1 = fms->add("Fu.ulam", "ulam 1;\nuse System;\nelement Fu {\nSystem s;\nInt(3) n;\n Int test(){\n n = -4;\n/* s.print(n);\n*/ s.print( n.minof);\n return 0;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Fu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3288_test_compiler_negconstant)

} //end MFM
