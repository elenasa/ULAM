#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3277_test_compiler_castatom2element_issue)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 0
      */
      //problem with eval answer is that an atom type appears the same as tu.
      return std::string("Exit status: 2\nUe_Tu { System s();  Int(32) me(2);  Int(32) test() {  Tu t;  Atom(96) a;  a Tu is cond { t a cast = me 2 cast = } if s ( me )print . me return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // FAILS BAD_CAST (of course it is! we protect against this sort of thing..)
      //bool rtn1 = fms->add("Tu.ulam", "ulam 1;\n element Tu {\nInt test(){\n Tu t;\nAtom a;\n t = (Tu) a;\n return 0;\n}\n}\n");

      // OK..using 'is' or 'as'
      bool rtn1 = fms->add("Tu.ulam", "ulam 1;\nuse System;\n element Tu {\nSystem s;\nInt me;\nInt test(){\nTu t;\nAtom a; if(a is Tu){\n t = (Tu) a;\n me = 2;\n }\n s.print(me);\n return me;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Tu.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3277_test_compiler_castatom2element_issue)

} //end MFM
