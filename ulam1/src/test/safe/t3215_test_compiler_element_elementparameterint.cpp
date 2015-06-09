#include "TestCase_EndToEndCompiler.h"

namespace MFM {

  BEGINTESTCASECOMPILER(t3215_test_compiler_element_elementparameterint)
  {
    std::string GetAnswerKey()
    {
      // element parameter chance not stored (as static variable) for eval
      return std::string("Exit status: 0\nUe_Foo { System s();  Bool(7) sp(false);  Int(32) a(0);  Int(32) test() {  Foo f;  s ( chance )print . a f chance . = a return } }\nUq_System { <NOMAIN> }\n");
    }

    std::string PresetTest(FileManagerString * fms)
    {
      // EP no longer storeintoable/
      bool rtn1 = fms->add("Foo.ulam","ulam 1;\nuse System;\n element Foo {\nSystem s;\nBool(7) sp;\nelement Int chance;\nInt a;\n Int test() {\n Foo f;\n/* f.chance = 1;\n */ s.print(chance);\n a = f.chance;\nreturn a;\n }\n }\n");

      // test system quark with native overloaded print funcs; assert
      bool rtn3 = fms->add("System.ulam", "ulam 1;\nquark System {\nVoid print(Unsigned arg) native;\nVoid print(Int arg) native;\nVoid print(Int(4) arg) native;\nVoid print(Int(3) arg) native;\nVoid print(Unary(3) arg) native;\nVoid print(Bool(3) arg) native;\nVoid assert(Bool b) native;\n}\n");

      if(rtn1 && rtn3)
	return std::string("Foo.ulam");

      return std::string("");
    }
  }

  ENDTESTCASECOMPILER(t3215_test_compiler_element_elementparameterint)

} //end MFM
