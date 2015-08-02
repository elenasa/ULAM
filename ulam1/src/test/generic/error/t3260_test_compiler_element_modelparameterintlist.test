#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3260_test_compiler_element_modelparameterintlist)
  {
    std::string GetAnswerKey()
    {
      //./Foo.ulam:6:22: ERROR: Missing ';' after model parameter definition 'chance'; Lists not supported.
      // model parameter chance not stored (as static variable) for eval
      return std::string("Exit status: 3\nUe_Foo { Int(8) b(2);  System s();  Bool(7) sp(false);  Int(8) a(1);  Int(8) c(3);  Int(32) test() {  Foo f;  f chance . 1 = s ( chance )print . a f chance . cast = f choice . 2 = s ( choice )print . b f choice . cast = f result . 3 = s ( result )print . c f result . cast = f chance . return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\nBool(7) sp;\nparameter Int chance = 1, choice = 2, result = 3;\nInt(8) a, b, c;\n Int test() {\n Foo f;\ns.print(chance);\na = f.chance;\ns.print(choice);\nb = f.choice;\ns.print(result);\nc = f.result;\nreturn f.chance;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3260_test_compiler_element_modelparameterintlist)

} //end MFM
