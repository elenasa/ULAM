#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3315_test_compiler_simplewithparens)
  {
    std::string GetAnswerKey()
    {
      return std::string("Exit status: 32\nUe_A { Bool(7) b(false);  System s();  Int(32) barne(32);  Int(32) test() {  barne 1 3 +b 8 * cast = s ( barne )print . barne return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("A.ulam","use System;\nelement A {\nSystem s;\nBool(7) b;\n Int barne;\n Int test() {\n barne = (1 + 3) * 8;\ns.print(barne);\n return barne;\n }\n }\n"); // case with parens

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("A.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3315_test_compiler_simplewithparens)

} //end MFM


