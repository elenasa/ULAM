#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3278_test_compiler_elementandquark_ep_issue)
  {
    std::string GetAnswerKey()
    {
      //problem with eval answer is that an atom type appears the same as tu.
      return std::string("Exit status: 0\nUe_Ebar { System s();  Int(32) test() {  s ( shouldBeOK )print . 0 cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Ebar.ulam", "ulam 1;\nuse System;\n element Ebar{\nSystem s;\nelement Int shouldBeOK;\nInt test(){\ns.print(shouldBeOK);\nreturn 0;\n }\n}\n");

      bool rtn2 = fms->add("Qfoo.ulam", "ulam 1;\nuse Ebar;\nquark Qfoo{\nVoid func(){\nEbar ebar;\n}\n}\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Ebar.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3278_test_compiler_elementandquark_ep_issue)

} //end MFM


