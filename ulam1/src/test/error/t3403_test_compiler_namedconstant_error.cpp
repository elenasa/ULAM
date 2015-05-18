#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3403_test_compiler_namedconstant_error)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 4
       */
      //different casts since Constants have explicit types
      // constant fold: j 1 7 +b =, 1 6u cast +b
      //./A.ulam:7:10: Warning: Attempting to fit named constant's value (cOW = 7) into a smaller bit size type: Int(3).
      return std::string("Exit status: 4\nUe_A { Int(32) j(4);  System s();  Int(32) test() {  3 = cOW const j 4 = s ( j )print . j return } }\nUq_System { <NOMAIN> }\nUq_B { typedef Int(3) Bar[2];  Int(3) j[2](0,0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by 3324, except constant type is 3-bits only
      bool rtn1 = fms->add("A.ulam","use System;\nuse B;\nelement A{\nSystem s;\nInt j;\nInt test () {\nconstant Int(3) cOW = 1 + B.sizeof;\n j = 1 + cOW;\ns.print(j);\n return j;\n}\n}\n");

      bool rtn2 = fms->add("B.ulam","quark B{\ntypedef Int(3) Bar[2]; Bar j;\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3403_test_compiler_namedconstant_error)

} //end MFM
