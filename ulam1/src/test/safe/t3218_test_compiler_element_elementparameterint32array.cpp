#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3218_test_compiler_element_elementparameterint32array)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 3
      */
      //fails because no eval storage for element parameters (e.g. chance)
      return std::string("Ue_Foo { System s();  Bool(7) sp(false);  Bool(1) last(false);  Int(32) test() {  Foo f;  f chance 1 [] . 3 cast = s ( chance 1 [] )print . chance 1 [] return } }\nExit status: 0");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\nBool(7) sp;\nelement Int chance[2];\nBool(1) last;\nInt test() {\n Foo f;\nf.chance[1] = 3;\ns.print(chance[1]);\nreturn chance[1];\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3218_test_compiler_element_elementparameterint32array)

} //end MFM


