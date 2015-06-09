#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3219_test_compiler_element_elementparameterint32arrayexpression)
  {
    std::string GetAnswerKey()
    {
      /* gen code output:
	 Int Arg: 0
      */

      //fails because no eval storage for element parameters (e.g. chance)
      return std::string("Exit status: -1\nUe_Foo { System s();  Bool(1) last(false);  Int(32) test() {  Foo f;  Int(32) i;  i 1 = s ( chance i [] )print . f chance i [] . return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // EP not soreintoable; commented out.
      //./Foo.ulam:12:13: ERROR: Invalid lefthand side of equals: <.>, type: Int(32).
      // f.chance[i] doesn't know about local variable i..should! fixed.
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\nelement Int chance[2];\nBool(1) last;\nInt test() {\n Foo f;\nInt i;\n i = 1;\n /* f.chance[i] = 3;\n*/ s.print(chance[i]);\nreturn f.chance[i];\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3219_test_compiler_element_elementparameterint32arrayexpression)

} //end MFM
