#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3217_test_compiler_elementandquark_elementparameterquark)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Bool(3) Arg: 0x0 (false)
      */
      return std::string("Exit status: -1\nUe_Foo { Bool(3) b(false);  System s();  Bool(1) last(false);  Int(32) test() {  b barchance valb 1 [] . cast = s ( b )print . b cast return } }\nUq_System { <NOMAIN> }\nUq_Bar { Bool(1) valb[3](false,false,false);  <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      //can't store into EP! commented out here
      //./Foo.ulam:10:19: ERROR: Invalid lefthand side of equals: <.>, type: Bool(1).
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\nuse Bar;\nelement Foo {\nSystem s;\nBool(3) b;\n element Bar barchance;\nBool last;\nInt test() {\n/* barchance.valb[1] = true;\n*/ b = barchance.valb[1];\ns.print(b);\nreturn b;\n }\n }\n");

      bool rtn2 = fms->add("Bar.ulam"," ulam 1;\n quark Bar {\n Bool valb[3];\n  Void reset(Bool b) {\n b = 0;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn2 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3217_test_compiler_elementandquark_elementparameterquark)

} //end MFM
