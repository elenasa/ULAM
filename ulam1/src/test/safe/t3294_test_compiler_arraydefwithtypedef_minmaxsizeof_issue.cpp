#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3294_test_compiler_arraydefwithtypedef_minmaxsizeof_issue)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Unsigned Arg: 8
      */

      return std::string("Exit status: 1\nUe_Efoo { System s();  typedef Unsigned(3) PortId;  Bool(1) heardFrom[8](false,false,false,false,false,false,false,false);  Int(32) test() {  typedef Bool(3) B3;  Bool(3) b;  b true cast = s ( 8u )print . b cast return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // should be all right (note: [7 + 1] works)
      bool rtn1 = fms->add("Efoo.ulam","ulam 1;\nuse System;\nelement Efoo {\nSystem s;\ntypedef Unsigned(3) PortId;\n Bool heardFrom[PortId.maxof + 1];\n Int test(){\ntypedef Bool(PortId.sizeof) B3;\n B3 b = true;\ns.print(heardFrom.sizeof);\n return (Int) b;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Efoo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3294_test_compiler_arraydefwithtypedef_minmaxsizeof_issue)

} //end MFM
