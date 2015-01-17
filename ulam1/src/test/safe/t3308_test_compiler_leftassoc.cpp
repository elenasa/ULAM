#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3308_test_compiler_leftassoc)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 1\nUe_A { Bool(7) b(false);  System s();  Int(32) a(1);  Int(32) test() {  a 1 1 -b 1 +b cast = s ( a )print . a return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\n Int a;\n Int test() {\n a = 1 - 1 + 1;\ns.print(a);\n return a;\n }\n }\n"); // we want 1, not -1

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3308_test_compiler_leftassoc)

} //end MFM


