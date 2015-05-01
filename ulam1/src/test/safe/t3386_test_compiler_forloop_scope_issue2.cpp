#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3386_test_compiler_forloop_scope_issue2)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 2
	 Int Arg: 1
	 Int Arg: 0
      */
      return std::string("Exit status: 0\nUe_A { Bool(7) b(true);  System s();  Int(32) d(0);  Int(32) test() {  d 3 = Int(32) index;  { index 0 = index d < cond b index cast = _1: index 1 += while } { Int(32) index;  index 0 = index 3 < cond { d 1 -= s ( d )print . } _2: index 1 += while } d return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //informed by: WindowUtils, and 3266
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\nInt d;\nInt test(){\nd = 3;\nInt index;\n for(index = 0; index < d; ++index)\n b = index;\n  for(Int index = 0; index < 3; ++index){\n --d;\ns.print(d);\n }\n return d;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3386_test_compiler_forloop_scope_issue2)

} //end MFM
