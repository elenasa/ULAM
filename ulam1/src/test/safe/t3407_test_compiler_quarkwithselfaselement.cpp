#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3407_test_compiler_quarkwithselfaselement)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 1
	 Int Arg: 2
      */
      //eval self doesn't reflect correctly: is an atom for quark's function call hidden arg
      return std::string("Exit status: 0\nUe_ElT { Int(32) i(0);  Typo t();  Typo r();  Int(32) test() {  System s;  t ( )incr . s ( i )print . r ( )incr . s ( i )print . i return } }\nUq_Typo { <NOMAIN> }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn2 = fms->add("ElT.ulam", "ulam 1;\nuse Typo;\nuse System;\n element ElT {\nTypo t,r;\nInt i;\n Int test() {\nSystem s;\n t.incr();\n s.print(i);\n r.incr();\n s.print(i);\n return i;\n}\n }\n");

      bool rtn1 = fms->add("Typo.ulam", "ulam 1;\nquark Typo {\nVoid incr(){\n if(self as ElT) {\n self.i += 1;\n}\n}\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("ElT.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3407_test_compiler_quarkwithselfaselement)

} //end MFM
