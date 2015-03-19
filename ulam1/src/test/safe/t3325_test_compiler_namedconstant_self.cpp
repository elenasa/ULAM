#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3325_test_compiler_namedconstant_self)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 34
       */
      //different casts since Constants have explicit types
      //constant fold: j 1 33 +b
      return std::string("Exit status: 34\nUe_A { System s();  Int(32) j(34);  Int(32) test() {  1 32u cast +b = cOW const j 34 = s ( j )print . j return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // works with either sizeof:
      //bool rtn1 = fms->add("A.ulam","use System;\nelement A{\nSystem s;\nInt j;\nInt test () {\nconstant Int cOW = 1 + A.sizeof;\n j = 1 + cOW;\ns.print(j);\n return j;\n}\n}\n");
      bool rtn1 = fms->add("A.ulam","use System;\nelement A{\nSystem s;\nInt j;\nInt test () {\nconstant Int cOW = 1 + self.sizeof;\n j = 1 + cOW;\ns.print(j);\n return j;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3325_test_compiler_namedconstant_self)

} //end MFM
