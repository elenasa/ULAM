#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3324_test_compiler_namedconstant)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 8
       */
      //different casts since Constants have explicit types
      return std::string("Exit status: 8\nUe_A { Int(32) j(8);  System s();  Int(32) test() {  1 6u cast +b = cOW const j 1 cast 7 +b = s ( j )print . j return } }\nUq_System { <NOMAIN> }\nUq_B { typedef Int(3) Bar[2];  Int(3) j[2](0,0);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nuse B;\nelement A{\nSystem s;\nInt j;\nInt test () {\nconstant Int cOW = 1 + B.sizeof;\n j = 1 + cOW;\ns.print(j);\n return j;\n}\n}\n");

      //bool rtn2 = fms->add("B.ulam","quark B{\nInt(3) j;\n}\n");
      //bool rtn2 = fms->add("B.ulam","quark B{\ntypedef Int(3) Bar[2]; Bar j;\n}\n");
      bool rtn2 = fms->add("B.ulam","quark B{\ntypedef Int(3) Bar[2]; Bar j;\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3324_test_compiler_namedconstant)

} //end MFM
